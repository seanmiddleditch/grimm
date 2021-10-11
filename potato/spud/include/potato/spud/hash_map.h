// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_hash_table.h"
#include "concepts.h"
#include "hash.h"
#include "platform.h"
#include "span.h"
#include "utility.h"

#if UP_ARCH_INTEL
#    include <emmintrin.h>
#    include <tmmintrin.h>
#else
#    error "Unsupported architecture"
#endif

#include <bit>

namespace up {
    namespace _detail::hash_map {
        template <typename Key, typename Value>
        struct key_value {
            template <typename K, typename V>
            constexpr key_value(K&& k, V&& v) : key(std::forward<K>(k))
                                              , value(std::forward<V>(v)) {}

            Key key;
            Value value;
        };

        template <typename Key, typename Value>
        class kv_proxy {
        public:
            using value_type = key_value<Key const&, Value&>;

            constexpr kv_proxy() noexcept = default;
            constexpr explicit kv_proxy(key_value<Key, Value>* items, size_t index) noexcept
                : _engaged(index != hash_table::constants::SENTINEL) {
                if (_engaged) {
                    new (&_data.proxy) value_type{items[index].key, items[index].value};
                }
            }

            constexpr explicit operator bool() const noexcept { return _engaged; }

            constexpr value_type* operator->() noexcept { return &_data.proxy; }
            constexpr value_type const* operator->() const noexcept { return &_data.proxy; }

            constexpr value_type& operator*() noexcept { return _data.proxy; }
            constexpr value_type const& operator*() const noexcept { return _data.proxy; }

        private:
            bool _engaged = false;
            union Data {
                constexpr Data() noexcept {} // NOLINT(modernize-use-equals-default)

                value_type proxy;
            } _data;
        };

        template <typename Item, typename Key, typename Equality>
        struct kv_equals {
            bool constexpr operator()(Item const& item, Key const& key) const noexcept {
                return Equality{}(item.key, key);
            }
        };

    } // namespace _detail::hash_map

    /// @brief Hash Map
    /// @tparam Key Key type.
    /// @tparam Value Value type.
    /// @tparam Hash Hash functor for Key.
    /// @tparam Equality Equality functor for Key.
    ///
    /// Hash Map based on Abseil's SwissTable (https://abseil.io/about/design/swisstables).
    ///
    /// See also the Hashbrown (Rust's hash_map) description at https://gankra.github.io/blah/hashbrown-tldr/.
    ///
    /// Our goal here is to provide a reasonably fast hash table, not necessary _the fastest_ hash table.
    /// Another goal is to keep the implementation relatively simple and light, meaning we avoid some
    /// potential performance improvements in exchange for a smaller and faster-to-instantiate type.
    ///
    /// This implementation is simplified in a few ways. First, we only support aligned groups, with the
    /// intent of removing the corner cases for accessing slots near the end of the table. Google's benchmarks
    /// show that supporting the floating group windows offers a ~10% read improvement with a ~-5% write
    /// regression.
    ///
    /// Our non-SSE fallback is also far less optimized, as we only intend to support SSE at this time.
    /// The portable implementation of SwissTable also changes some bits of layout (e.g. group width) and
    /// supporting this complicates the code.
    ///
    template <typename Key, typename Value, typename Hash = uhash<>, typename Equality = equality>
    class hash_map {
    public:
        using key_type = Key;
        using mapped_type = Value;
        using value_type = _detail::hash_map::key_value<key_type const, mapped_type>;
        using size_type = size_t;
        using hash_type = hash_result_t<Hash, Key>;

        struct proxy_type : _detail::hash_map::kv_proxy<Key, Value> {
            using _detail::hash_map::kv_proxy<Key, Value>::kv_proxy;
        };
        struct const_proxy_type : _detail::hash_map::kv_proxy<Key, Value const> {
            using _detail::hash_map::kv_proxy<Key, Value const>::kv_proxy;
        };

        constexpr hash_map() noexcept = default;
        constexpr ~hash_map() noexcept {
            clear();
            _drop();
        }

        constexpr hash_map(hash_map&& rhs) noexcept
            : _size(rhs._size)
            , _groups(rhs._groups)
            , _control(rhs._control)
            , _items(rhs._items) {
            rhs._groups = rhs._size = 0;
            rhs._control = nullptr;
            rhs._items = nullptr;
        }
        constexpr hash_map& operator=(hash_map&& rhs) noexcept;

        [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }
        [[nodiscard]] constexpr size_type size() const noexcept { return _size; }
        [[nodiscard]] constexpr size_type capacity() const noexcept {
            return _groups * _detail::hash_table::constants::WIDTH;
        }

        [[nodiscard]] constexpr float load_factor() const noexcept {
            return static_cast<float>(_size) / static_cast<float>(capacity());
        }

        [[nodiscard]] constexpr float max_load_factor() const noexcept {
            return static_cast<float>(_groups * _detail::hash_table::constants::GROUP_LOAD) /
                static_cast<float>(capacity());
        }

        constexpr void clear() noexcept;

        template <convertible_to<Key> FindKey = Key>
        [[nodiscard]] constexpr bool contains(FindKey const& key) const noexcept {
            return _find(key, Hash{}(key)) != _detail::hash_table::constants::SENTINEL;
        }

        template <convertible_to<Key> FindKey = Key>
        [[nodiscard]] constexpr proxy_type find(FindKey const& key) noexcept {
            auto const index = _find(key, Hash{}(key));
            return proxy_type{_items, index};
        }

        template <convertible_to<Key> InsertKey = Key, convertible_to<Value> InsertValue = Value>
        constexpr bool insert(InsertKey&& key, InsertValue&& value);

        template <convertible_to<Key> EraseKey = Key>
        constexpr bool erase(EraseKey const& key) noexcept;

    private:
        using item_type = _detail::hash_map::key_value<Key, Value>;
        using control_type = int8;
        using match_ops = _detail::hash_table::match_ops_sse;
        using memory_ops = _detail::hash_table::memory_ops<item_type>;
        using hash_ops = _detail::hash_table::hash_ops<hash_type>;
        using table_ops = _detail::hash_table::table_ops<
            item_type,
            Key,
            Hash,
            _detail::hash_map::kv_equals<item_type, Key, Equality>,
            hash_ops,
            match_ops>;

        constexpr void _drop() noexcept;
        constexpr void _grow();
        template <convertible_to<Key> FindKey>
        constexpr size_type _find(FindKey const& key, hash_type hash) const noexcept;
        template <convertible_to<Key> InsertKey, convertible_to<Value> InsertValue>
        constexpr bool _insert(InsertKey&& key, InsertValue&& value, hash_type hash);

        size_type _size = 0;
        size_type _groups = 0;
        control_type* _control = nullptr;
        item_type* _items = nullptr;
    };

    template <typename Key, typename Value, typename Hash, typename Equality>
    constexpr hash_map<Key, Value, Hash, Equality>& hash_map<Key, Value, Hash, Equality>::operator=(
        hash_map&& rhs) noexcept {
        if (_control != rhs._control) {
            clear();
            _drop();
            _size = rhs._size;
            _groups = rhs._groups;
            _control = rhs._control;
            _items = rhs._items;

            rhs._size = rhs._groups = 0;
            rhs._control = nullptr;
            rhs._items = nullptr;
        }
        return *this;
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    constexpr void hash_map<Key, Value, Hash, Equality>::clear() noexcept {
        if (_size == 0) {
            [[unlikely]] return;
        }

        if (table_ops::clear(_groups, _control, _items)) {
            _drop();
        }

        _size = 0;
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    template <convertible_to<Key> InsertKey, convertible_to<Value> InsertValue>
    constexpr bool hash_map<Key, Value, Hash, Equality>::insert(InsertKey&& key, InsertValue&& value) {
        auto const hash = Hash{}(key);

        if (_size >= _groups * _detail::hash_table::constants::GROUP_LOAD) {
            _grow();
        }

        return _insert(key, value, hash);
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    template <convertible_to<Key> EraseKey>
    constexpr bool hash_map<Key, Value, Hash, Equality>::erase(EraseKey const& key) noexcept {
        if (_size == 0) {
            return false;
        }

        auto const hash = Hash{}(key);
        if (table_ops::erase(_groups, _control, _items, key, hash)) {
            --_size;
            return true;
        }

        return false;
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    constexpr void hash_map<Key, Value, Hash, Equality>::_drop() noexcept {
        memory_ops::deallocate(_groups, _control);

        _groups = 0;
        _control = nullptr;
        _items = nullptr;
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    constexpr void hash_map<Key, Value, Hash, Equality>::_grow() {
        using constants = _detail::hash_table::constants;

        // we guarantee that this leaves us in an empty state
        hash_map temp = std::move(*this);

        _groups = temp._groups != 0 ? temp._groups << 1 : 1;

        memory_ops::allocate(_groups, _control, _items);

        if (temp._groups != 0) {
            auto const oldCapacity = temp.capacity();
            for (size_type index = 0; index != oldCapacity; ++index) {
                if ((temp._control[index] & constants::EMPTY) == 0) {
                    insert(std::move(temp._items[index].key), std::move(temp._items[index].value));
                }
            }
        }
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    template <convertible_to<Key> FindKey>
    constexpr auto hash_map<Key, Value, Hash, Equality>::_find(FindKey const& key, hash_type hash) const noexcept
        -> size_type {
        if (_size == 0) {
            [[unlikely]] return _detail::hash_table::constants::SENTINEL;
        }

        return table_ops::find(_groups, _control, _items, key, hash);
    }

    template <typename Key, typename Value, typename Hash, typename Equality>
    template <convertible_to<Key> InsertKey, convertible_to<Value> InsertValue>
    constexpr bool hash_map<Key, Value, Hash, Equality>::_insert(InsertKey&& key, InsertValue&& value, hash_type hash) {
        if (auto const index = _find(key, hash); index != _detail::hash_table::constants::SENTINEL) {
            _items[index].value = std::forward<InsertValue>(value);
            return false;
        }

        auto const index = table_ops::findEmptyOrTombstone(_groups, _control, hash);

        new (&_items[index]) item_type(std::forward<InsertKey>(key), std::forward<InsertValue>(value));
        _control[index] = hash_ops::_h2(hash);
        ++_size;
        return true;
    }
} // namespace up
