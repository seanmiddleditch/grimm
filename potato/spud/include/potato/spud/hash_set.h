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

    /// @brief Hash Set
    /// @tparam Value Value type.
    /// @tparam Hash Hash functor for Value.
    /// @tparam Equality Equality functor for Value.
    ///
    template <typename Value, typename Hash = uhash<>, typename Equality = equality>
    class hash_set {
    public:
        using value_type = Value const;
        using size_type = size_t;
        using hash_type = hash_result_t<Hash, Value>;

        constexpr hash_set() noexcept = default;
        constexpr ~hash_set() noexcept {
            clear();
            _drop();
        }

        constexpr hash_set(hash_set&& rhs) noexcept
            : _size(rhs._size)
            , _groups(rhs._groups)
            , _control(rhs._control)
            , _items(rhs._items) {
            rhs._groups = rhs._size = 0;
            rhs._control = nullptr;
            rhs._items = nullptr;
        }
        constexpr hash_set& operator=(hash_set&& rhs) noexcept;

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

        template <convertible_to<Value> FindValue = Value>
        [[nodiscard]] constexpr bool contains(FindValue const& value) const noexcept {
            return _find(value, Hash{}(value)) != _detail::hash_table::constants::SENTINEL;
        }

        template <convertible_to<Value> FindValue = Value>
        [[nodiscard]] constexpr Value const* find(FindValue const& value) noexcept {
            using constants = _detail::hash_table::constants;
            auto const index = _find(value, Hash{}(value));
            return index != constants::SENTINEL ? &_items[index] : nullptr;
        }

        template <convertible_to<Value> InsertValue = Value>
        constexpr bool insert(InsertValue&& value);

        template <convertible_to<Value> EraseValue = Value>
        constexpr bool erase(EraseValue const& value) noexcept;

    private:
        using item_type = Value;
        using control_type = int8;
        using match_ops = _detail::hash_table::match_ops_sse;
        using memory_ops = _detail::hash_table::memory_ops<item_type>;
        using hash_ops = _detail::hash_table::hash_ops<hash_type>;
        using table_ops = _detail::hash_table::table_ops<item_type, Value, Hash, Equality, hash_ops, match_ops>;

        constexpr void _drop() noexcept;
        constexpr void _grow();
        template <convertible_to<Value> FindValue>
        constexpr size_type _find(FindValue const& value, hash_type hash) const noexcept;
        template <convertible_to<Value> InsertValue>
        constexpr bool _insert(InsertValue&& value, hash_type hash);

        size_type _size = 0;
        size_type _groups = 0;
        control_type* _control = nullptr;
        item_type* _items = nullptr;
    };

    template <typename Value, typename Hash, typename Equality>
    constexpr hash_set<Value, Hash, Equality>& hash_set<Value, Hash, Equality>::operator=(hash_set&& rhs) noexcept {
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

    template <typename Value, typename Hash, typename Equality>
    constexpr void hash_set<Value, Hash, Equality>::clear() noexcept {
        if (_size == 0) {
            [[unlikely]] return;
        }

        if (table_ops::clear(_groups, _control, _items)) {
            _drop();
        }

        _size = 0;
    }

    template <typename Value, typename Hash, typename Equality>
    template <convertible_to<Value> InsertValue>
    constexpr bool hash_set<Value, Hash, Equality>::insert(InsertValue&& value) {
        auto const hash = Hash{}(value);

        if (_size >= _groups * _detail::hash_table::constants::GROUP_LOAD) {
            _grow();
        }

        return _insert(value, hash);
    }

    template <typename Value, typename Hash, typename Equality>
    template <convertible_to<Value> EraseValue>
    constexpr bool hash_set<Value, Hash, Equality>::erase(EraseValue const& value) noexcept {
        if (_size == 0) {
            return false;
        }

        auto const hash = Hash{}(value);
        if (table_ops::erase(_groups, _control, _items, value, hash)) {
            --_size;
            return true;
        }

        return false;
    }

    template <typename Value, typename Hash, typename Equality>
    constexpr void hash_set<Value, Hash, Equality>::_drop() noexcept {
        memory_ops::deallocate(_groups, _control);

        _groups = 0;
        _control = nullptr;
        _items = nullptr;
    }

    template <typename Value, typename Hash, typename Equality>
    constexpr void hash_set<Value, Hash, Equality>::_grow() {
        using constants = _detail::hash_table::constants;

        // we guarantee that this leaves us in an empty state
        hash_set temp = std::move(*this);

        _groups = temp._groups != 0 ? temp._groups << 1 : 1;

        memory_ops::allocate(_groups, _control, _items);

        if (temp._groups != 0) {
            auto const oldCapacity = temp.capacity();
            for (size_type index = 0; index != oldCapacity; ++index) {
                if ((temp._control[index] & constants::EMPTY) == 0) {
                    insert(std::move(temp._items[index]));
                }
            }
        }
    }

    template <typename Value, typename Hash, typename Equality>
    template <convertible_to<Value> FindValue>
    constexpr auto hash_set<Value, Hash, Equality>::_find(FindValue const& value, hash_type hash) const noexcept
        -> size_type {
        if (_size == 0) {
            [[unlikely]] return _detail::hash_table::constants::SENTINEL;
        }

        return table_ops::find(_groups, _control, _items, value, hash);
    }

    template <typename Value, typename Hash, typename Equality>
    template <convertible_to<Value> InsertValue>
    constexpr bool hash_set<Value, Hash, Equality>::_insert(InsertValue&& value, hash_type hash) {
        if (auto const index = _find(value, hash); index != _detail::hash_table::constants::SENTINEL) {
            return false;
        }

        auto const index = table_ops::findEmptyOrTombstone(_groups, _control, hash);

        new (&_items[index]) item_type(std::forward<InsertValue>(value));
        _control[index] = hash_ops::_h2(hash);
        ++_size;
        return true;
    }
} // namespace up
