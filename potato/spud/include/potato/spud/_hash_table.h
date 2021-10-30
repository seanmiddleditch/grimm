// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "hash.h"
#include "platform.h"
#include "utility.h"

#if UP_ARCH_INTEL
#    include <emmintrin.h>
#    include <tmmintrin.h>
#else
#    error "Unsupported architecture"
#endif

#include <bit>
#include <cstring>

namespace up::_detail::hash_table {
    struct constants {
        static constexpr int8 SPECIAL = static_cast<int8>(0xff);
        static constexpr int8 EMPTY = static_cast<int8>(0x80);
        static constexpr int8 TOMBSTONE = static_cast<int8>(0xfe);

        static constexpr size_t SENTINEL = ~size_t{0};

        static constexpr int WIDTH = 16;
        static constexpr int GROUP_LOAD = WIDTH - WIDTH / 8;

        static constexpr uint8 H1_MASK = 0x7f;
        static constexpr int H2_SHIFT = 7;

        static constexpr uint8 MEM_FREE = 0xdd;

        static constexpr size_t DROP_THRESHOLD = 127;
    };

    template <typename Key, typename Value>
    struct key_value {
        Key key;
        Value value;
    };

    struct match_ops_sse {
        static inline unsigned match(int8 const* control, int8 match) noexcept;
        static inline unsigned matchEmpty(int8 const* control) noexcept;
        static inline unsigned matchEmptyOrTombstone(int8 const* control) noexcept;
        static inline unsigned matchFull(int8 const* control) noexcept;
    };

    template <typename Item>
    struct memory_ops {
        static constexpr size_t alignment = alignof(Item) > constants::WIDTH ? alignof(Item) : constants::WIDTH;

        static constexpr size_t padding(size_t groups) noexcept;
        static constexpr size_t size(size_t groups) noexcept;

        static void allocate(size_t groups, int8*& out_control, Item*& out_items);
        static void deallocate(size_t groups, void* data);
    };

    template <typename Hash>
    struct hash_ops {
        static constexpr Hash _h1(Hash hash) noexcept { return hash >> constants::H2_SHIFT; }
        static constexpr int8 _h2(Hash hash) noexcept { return static_cast<int8>(hash & constants::H1_MASK); }
    };

    template <typename Item, typename Key, typename Hash, typename Equality, typename HashOps, typename MatchOps>
    struct table_ops {
        static constexpr size_t find(
            size_t groups,
            int8 const* control,
            Item const* items,
            Key const& key,
            hash_result_t<Hash, Key> hash) noexcept;
        static constexpr size_t findEmptyOrTombstone(
            size_t groups,
            int8 const* control,
            hash_result_t<Hash, Key> hash) noexcept;
        static constexpr bool erase(
            size_t groups,
            int8* control,
            Item* items,
            Key const& key,
            hash_result_t<Hash, Key> hash) noexcept;

        // returns true if the caller must drop allocated memory (it won't be cleared/reset)
        static bool clear(size_t groups, int8* control, Item* items) noexcept;
    };

    unsigned match_ops_sse::match(int8 const* control, int8 match) noexcept {
        auto const* const slots = reinterpret_cast<__m128i const*>(control);
        return _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(slots), _mm_set1_epi8(match)));
    }

    unsigned match_ops_sse::matchEmpty(int8 const* control) noexcept {
        auto const* const slots = reinterpret_cast<__m128i const*>(control);
        auto const mask = _mm_loadu_si128(slots);
        return _mm_movemask_epi8(_mm_sign_epi8(mask, mask));
    }

    unsigned match_ops_sse::matchEmptyOrTombstone(int8 const* control) noexcept {
        auto const* const slots = reinterpret_cast<__m128i const*>(control);
        return _mm_movemask_epi8(_mm_cmpgt_epi8(_mm_set1_epi8(constants::SPECIAL), _mm_loadu_si128(slots)));
    }

    unsigned match_ops_sse::matchFull(int8 const* control) noexcept { return ~matchEmptyOrTombstone(control) & 0xffff; }

    template <typename Item>
    constexpr auto memory_ops<Item>::padding(size_t groups) noexcept -> size_t {
        size_t const control = sizeof(int8) * groups * constants::WIDTH;
        return control % alignof(Item);
    }

    template <typename Item>
    constexpr auto memory_ops<Item>::size(size_t groups) noexcept -> size_t {
        size_t const control = sizeof(int8) * groups * constants::WIDTH;
        size_t const items = sizeof(Item) * groups * constants::WIDTH;
        return control + items + padding(groups);
    }

    template <typename Item>
    void memory_ops<Item>::allocate(size_t groups, int8*& out_control, Item*& out_items) {
        using constants = constants;

        auto const bytes = size(groups);
        auto const slots = groups * constants::WIDTH;
        auto const itemsOffset = sizeof(int8) * slots + padding(groups);

        void* const data = operator new(bytes, std::align_val_t(alignment));

        out_control = static_cast<int8*>(data);
        out_items = reinterpret_cast<Item*>(static_cast<char*>(data) + itemsOffset);

        std::memset(out_control, constants::EMPTY, sizeof(int8) * slots);
    }

    template <typename Item>
    void memory_ops<Item>::deallocate(size_t groups, void* data) {
        ::operator delete(data, size(groups), std::align_val_t(alignment));
    }

    template <typename Item, typename Key, typename Hash, typename Equality, typename HashOps, typename MatchOps>
    constexpr auto table_ops<Item, Key, Hash, Equality, HashOps, MatchOps>::find(
        size_t groups,
        int8 const* control,
        Item const* items,
        Key const& key,
        hash_result_t<Hash, Key> hash) noexcept -> size_t {
        using constants = constants;

        size_t group = HashOps::_h1(hash) & (groups - 1);
        auto const h2 = HashOps::_h2(hash);
        size_t probe = 1;

        for (;;) {
            auto const alignedIndex = group * constants::WIDTH;

            unsigned matches = MatchOps::match(control + alignedIndex, h2);

            // iterate set bits
            while (matches != 0) {
                size_t index = alignedIndex + std::countr_zero(matches);

                if (Equality{}(items[index], key)) {
                    return index;
                }

                matches &= matches - 1;
            }

            // if there's any empty slot, we don't have to probe forward
            if (MatchOps::matchEmpty(control + alignedIndex) != 0) {
                return constants::SENTINEL;
            }

            // quadratic probe
            group = (group + probe++) & (groups - 1);
        }
    }

    template <typename Item, typename Key, typename Hash, typename Equality, typename HashOps, typename MatchOps>
    constexpr auto table_ops<Item, Key, Hash, Equality, HashOps, MatchOps>::findEmptyOrTombstone(
        size_t groups,
        int8 const* controls,
        hash_result_t<Hash, Key> hash) noexcept -> size_t {
        size_t group = HashOps::_h1(hash) & (groups - 1);
        size_t probe = 1;

        for (;;) {
            auto const alignedIndex = group * constants::WIDTH;

            unsigned matches = MatchOps::matchEmptyOrTombstone(controls + alignedIndex);

            // set first empty or tombstone slot to the value, if we have one
            if (matches != 0) {
                return alignedIndex + std::countr_zero(matches);
            }

            // quadratic probe - if we had an empty slot, we'd have filled it
            group = (group + probe++) & (groups - 1);
        }
    }

    template <typename Item, typename Key, typename Hash, typename Equality, typename HashOps, typename MatchOps>
    constexpr bool table_ops<Item, Key, Hash, Equality, HashOps, MatchOps>::erase(
        size_t groups,
        int8* control,
        Item* items,
        Key const& key,
        hash_result_t<Hash, Key> hash) noexcept {
        using constants = constants;

        auto const index = find(groups, control, items, key, hash);
        if (index == constants::SENTINEL) {
            [[unlikely]] return false;
        }

        // if we have other empty slots in the group, we can mark this erased
        // item as empty, since we weren't part of a probe chain anyway; otherwise
        // our group was possibly part of a probe chain so we must leave a TOMBSTONE
        auto const alignedIndex = index & ~(constants::WIDTH - 1);
        bool const hasEmpty = MatchOps::matchEmpty(control + alignedIndex) != 0;

        control[index] = hasEmpty ? constants::EMPTY : constants::TOMBSTONE;
        items[index].~Item();

#if !defined(NDEBUG)
        std::memset(reinterpret_cast<char*>(&items[index]), constants::MEM_FREE, sizeof(Item));
#endif

        return true;
    }

    template <typename Item, typename Key, typename Hash, typename Equality, typename HashOps, typename MatchOps>
    bool table_ops<Item, Key, Hash, Equality, HashOps, MatchOps>::clear(
        size_t groups,
        int8* control,
        Item* items) noexcept {
        using constants = constants;

        auto const shouldDrop = groups >= constants::DROP_THRESHOLD;

        if constexpr (!std::is_trivially_destructible_v<Item>) {
            for (size_t alignedIndex = 0, slots = groups * constants::WIDTH; alignedIndex != slots;
                 alignedIndex += constants::WIDTH) {
                unsigned matches = MatchOps::matchFull(control + alignedIndex);

                // iterate set bits
                while (matches != 0) {
                    size_t const index = alignedIndex + std::countr_zero(matches);

                    control[index] = constants::EMPTY;
                    items[index].~Item();

#if !defined(NDEBUG)
                    std::memset(reinterpret_cast<char*>(&items[index]), constants::MEM_FREE, sizeof(Item));
#endif

                    matches &= matches - 1;
                }
            }
        }
        else {
#if !defined(NDEBUG)
            if (!shouldDrop) {
                std::memset(
                    reinterpret_cast<char*>(items),
                    constants::MEM_FREE,
                    sizeof(Item) * groups * constants::WIDTH);
            }
#endif
        }

        // only clear control if our size is small, otherwise clearing
        // is more expensive than just re-allocating
        if (!shouldDrop) {
            std::memset(control, constants::EMPTY, sizeof(int8) * groups * constants::WIDTH);
        }

        return shouldDrop;
    }
} // namespace up::_detail::hash_table
