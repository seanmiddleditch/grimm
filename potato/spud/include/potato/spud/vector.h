// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if !defined(DOXYGEN_SHOULD_SKIP_THIS) // Doxygen can't handle our use of C++20 concepts currently

#    include "_assertion.h"
#    include "int_types.h"
#    include "memory_util.h"
#    include "numeric_util.h"
#    include "span.h"
#    include "traits.h"

#    include <initializer_list>
#    include <new>
#    include <type_traits>

namespace up {
    template <typename T>
    class vector;

    template <typename T>
    vector(span<T>) -> vector<std::remove_const_t<T>>;
    template <typename T>
    vector(std::initializer_list<T>) -> vector<std::remove_const_t<T>>;
    template <typename IteratorT, typename SentinelT>
    vector(IteratorT, SentinelT) -> vector<std::remove_const_t<decltype(*std::declval<IteratorT>())>>;

    template <typename T>
    class vector {
    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = T const*;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using rvalue_reference = T&&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        vector() noexcept = default;

        template <typename IteratorT, typename SentinelT>
        inline explicit vector(IteratorT begin, SentinelT end) requires std::is_constructible_v<T, deref_t<IteratorT>>;
        template <typename InsertT>
        inline /*implicit*/ vector(
            std::initializer_list<InsertT> initial) requires std::is_constructible_v<T, InsertT const>;
        inline explicit vector(size_type size, const_reference initial) requires std::is_copy_constructible_v<T>;
        template <typename InsertT>
        inline explicit vector(span<InsertT> source) requires std::is_constructible_v<T, InsertT>;
        inline explicit vector(size_type size) requires std::is_default_constructible_v<T>;

        vector(vector const&) = delete;
        vector& operator=(vector const&) = delete;

        inline vector(vector&& src) noexcept;
        inline vector& operator=(vector&& src) noexcept;

        inline ~vector();

        static vector acquire(T* memory, size_type count) noexcept;

        iterator begin() noexcept { return _first; }
        iterator end() noexcept { return _last; }

        const_iterator begin() const noexcept { return _first; }
        const_iterator end() const noexcept { return _last; }

        const_iterator cbegin() const noexcept { return _first; }
        const_iterator cend() const noexcept { return _last; }

        pointer data() noexcept { return _first; }
        const_pointer data() const noexcept { return _first; }

        pointer release() noexcept;

        bool empty() const noexcept { return _first == _last; }
        size_type size() const noexcept { return _last - _first; }
        size_type capacity() const noexcept { return _sentinel - _first; }

        reference operator[](size_type index) noexcept { return _first[index]; }
        const_reference operator[](size_type index) const noexcept { return _first[index]; }

        reference front() noexcept { return *_first; }
        const_reference front() const noexcept { return *_first; }

        reference back() noexcept { return *(_last - 1); }
        const_reference back() const noexcept { return *(_last - 1); }

        void reserve(size_type required);
        void resize(size_type new_size);
        void resize(size_type new_size, const_reference init);
        void clear() noexcept;
        void shrink_to_fit() noexcept;

        template <typename... ParamsT>
        auto emplace(const_iterator pos, ParamsT&&... params) -> reference
            requires std::is_constructible_v<T, ParamsT...>;
        auto emplace_back() -> reference requires std::is_default_constructible_v<T>;
        template <typename... ParamsT>
        auto emplace_back(ParamsT&&... params) -> reference requires std::is_constructible_v<T, ParamsT...>;

        template <typename InsertT = value_type>
        auto insert(const_iterator pos, InsertT&& value) -> reference
            requires std::is_constructible_v<T, decltype(value)> {
            return emplace(pos, std::forward<InsertT>(value));
        }

        template <typename IteratorT, typename SentinelT>
        iterator insert(const_iterator pos, IteratorT begin, SentinelT end);

        reference push_back(const_reference value) requires std::is_copy_constructible_v<T> {
            return emplace_back(value);
        }
        reference push_back(rvalue_reference value) requires std::is_move_constructible_v<T> {
            return emplace_back(std::move(value));
        }
        template <typename InsertT = value_type>
        reference push_back(InsertT&& value) requires std::is_constructible_v<T, decltype(value)> {
            return emplace_back(std::forward<InsertT>(value));
        }

        iterator erase(const_iterator pos);
        iterator erase(const_iterator begin, const_iterator end);

        span<T> subspan(size_type index) noexcept { return span<T>{_first + index, _last}; }
        span<T> subspan(size_type index, size_type count) noexcept {
            return span<T>{_first + index, _first + index + count};
        }

        span<T const> subspan(size_type index) const noexcept { return span<T const>{_first + index, _last}; }
        span<T const> subspan(size_type index, size_type count) const noexcept {
            return span<T const>{_first + index, _first + index + count};
        }

        operator span<T>() noexcept { return span<T>(_first, _last); }
        operator span<T const>() const noexcept { return span<T const>(_first, _last); }

        span<byte const> as_bytes() const noexcept {
            return span<byte const>(reinterpret_cast<byte const*>(_first), reinterpret_cast<byte const*>(_last));
        }

        void pop_back();

    private:
        T* _allocate(size_type capacity);
        void _deallocate(T* ptr, size_type capacity);
        size_type _grow(size_type minimum = 4);
        void _rshift(T* pos, size_type shift);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        static auto _to_iterator(const_iterator citer) noexcept -> iterator { return const_cast<iterator>(citer); }

        T* _first = nullptr;
        T* _last = nullptr;
        T* _sentinel = nullptr;
    };

    template <typename T>
    template <typename IteratorT, typename SentinelT>
    vector<T>::vector(IteratorT begin, SentinelT end) requires std::is_constructible_v<T, deref_t<IteratorT>> {
        insert(_first, begin, end);
    }

    template <typename T>
    template <typename InsertT>
    vector<T>::vector(std::initializer_list<InsertT> initial) requires std::is_constructible_v<T, InsertT const> {
        insert(_first, initial.begin(), initial.end());
    }

    template <typename T>
    vector<T>::vector(size_type size, const_reference initial) requires std::is_copy_constructible_v<T> {
        resize(size, initial);
    }

    template <typename T>
    template <typename InsertT>
    vector<T>::vector(span<InsertT> source) requires std::is_constructible_v<T, InsertT> {
        insert(_first, source.begin(), source.end());
    }

    template <typename T>
    vector<T>::vector(size_type size) requires std::is_default_constructible_v<T> {
        resize(size);
    }

    template <typename T>
    vector<T>::vector(vector&& src) noexcept : _first(src._first)
                                             , _last(src._last)
                                             , _sentinel(src._sentinel) {
        src._sentinel = src._last = src._first = nullptr;
    }

    template <typename T>
    vector<T>::~vector() {
        destruct_n(_first, _last - _first);
        _deallocate(_first, _sentinel - _first);
    }

    template <typename T>
    auto vector<T>::operator=(vector&& src) noexcept -> vector& {
        if (this != &src) {
            clear();
            shrink_to_fit();

            _first = src._first;
            _last = src._last;
            _sentinel = src._sentinel;

            src._sentinel = src._last = src._first = nullptr;
        }
        return *this;
    }

    template <typename T>
    auto vector<T>::acquire(T* memory, size_type count) noexcept -> vector {
        vector rs;
        rs._first = memory;
        rs._last = rs._sentinel = count;
        return rs;
    }

    template <typename T>
    T* vector<T>::release() noexcept {
        UP_SPUD_ASSERT(
            _last == _sentinel,
            "Releasing memory from a vector that has uninitialized capacity; call resize(capacity()) first!");
        T* tmp = _first;
        _first = _last = _sentinel = nullptr;
        return tmp;
    }

    template <typename T>
    T* vector<T>::_allocate(size_type capacity) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        return static_cast<T*>(operator new(capacity * sizeof(T), std::align_val_t(alignof(T))));
    }

    template <typename T>
    void vector<T>::_deallocate(T* ptr, size_type capacity) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        ::operator delete(ptr, capacity * sizeof(T), std::align_val_t(alignof(T)));
    }

    template <typename T>
    size_t vector<T>::_grow(size_t minimum) {
        size_type capacity = _sentinel - _first;
        capacity += capacity >> 1;

        return max(minimum, capacity); // grow by 50%
    }

    template <typename T>
    void vector<T>::_rshift(T* pos, size_t shift) {
        size_t size = _last - pos;

        // copy elements to the new area, as needed
        auto const tail = min(size, shift);
        unitialized_move_n(_last - tail, tail, _last + shift - tail);

        _last += shift;

        // shift elements inside the already-initialized parts of the vector
        auto const head = size - tail;
        move_backwards_n(pos, head, pos + head);
    }

    template <typename T>
    void vector<T>::reserve(size_type required) {
        size_type const capacity = _sentinel - _first;
        if (capacity < required) {
            T* tmp = _allocate(required);
            auto const count = _last - _first;
            unitialized_move_n(_first, count, tmp);
            destruct_n(_first, count);
            _deallocate(_first, _sentinel - _first);
            _first = tmp;
            _last = _first + count;
            _sentinel = _first + required;
        }
    }

    template <typename T>
    void vector<T>::resize(size_type new_size) {
        size_type const size = _last - _first;
        if (size < new_size) {
            reserve(new_size);
            default_construct_n(_last, new_size - size);
        }
        else if (size > new_size) {
            destruct_n(_first + new_size, size - new_size);
        }
        _last = _first + new_size;
    }

    template <typename T>
    void vector<T>::resize(size_type new_size, const_reference init) {
        size_type const size = _last - _first;
        if (size < new_size) {
            reserve(new_size);
            uninitialized_value_construct_n(_last, new_size - size, init);
        }
        else if (size > new_size) {
            destruct_n(_first + new_size, size - new_size);
        }
        _last = _first + new_size;
    }

    template <typename T>
    void vector<T>::clear() noexcept {
        destruct_n(_first, _last - _first);
        _last = _first;
    }

    template <typename T>
    void vector<T>::shrink_to_fit() noexcept {
        if (_sentinel == nullptr) { /* do nothing */
        }
        else if (_first == _last) {
            _deallocate(_first, _sentinel - _first);
            _first = _last = _sentinel = nullptr;
        }
        else if (_sentinel > _last) {
            auto const size = _last - _first;
            T* tmp = _allocate(size);
            unitialized_move_n(_first, size, tmp);
            destruct_n(_first, size);
            _deallocate(_first, _sentinel - _first);
            _first = tmp;
            _sentinel = _last = _first + size;
        }
    }

    template <typename T>
    template <typename... ParamsT>
    auto vector<T>::emplace(const_iterator pos, ParamsT&&... params) -> reference
        requires std::is_constructible_v<T, ParamsT...> {
        if (pos == _last) {
            return emplace_back(std::forward<ParamsT>(params)...);
        }

        if (_last < _sentinel) {
            iterator mpos = _to_iterator(pos);
            _rshift(mpos, 1);
            mpos->~value_type();
            new (mpos) value_type(std::forward<ParamsT>(params)...);
            return *mpos;
        }

        auto const size = _last - _first;
        auto const index = pos - _first;

        // grow
        auto const newCapacity = _grow(size + 1);
        T* tmp = _allocate(newCapacity);

        // insert new element
        new (tmp + index) value_type(std::forward<ParamsT>(params)...);

        // move over old elements
        unitialized_move_n(_first, index, tmp);
        unitialized_move_n(_first + index, size - index, tmp + index + 1);

        // free up old space
        destruct_n(_first, size);
        _deallocate(_first, _sentinel - _first);

        // commit new space
        _first = tmp;
        _last = _first + size + 1;
        _sentinel = _first + newCapacity;

        return _first[size];
    }

    template <typename T>
    auto vector<T>::emplace_back() -> reference requires std::is_default_constructible_v<T> {
        if (_last != _sentinel) {
            T* value = new (_last++) value_type;
            return *value;
        }

        auto const size = _last - _first;

        // grow
        auto const newCapacity = _grow(size + 1);
        T* tmp = _allocate(newCapacity);

        // insert new element
        new (tmp + size) value_type;

        // move over old elements
        unitialized_move_n(_first, size, tmp);

        // free up old space
        destruct_n(_first, size);
        _deallocate(_first, _sentinel - _first);

        // commit new space
        _first = tmp;
        _last = _first + size + 1;
        _sentinel = _first + newCapacity;

        return _first[size];
    }

    template <typename T>
    template <typename... ParamsT>
    auto vector<T>::emplace_back(ParamsT&&... params) -> reference requires std::is_constructible_v<T, ParamsT...> {
        if (_last != _sentinel) {
            T* value = new (_last++) value_type(std::forward<ParamsT>(params)...);
            return *value;
        }

        auto const size = _last - _first;

        // grow
        auto const newCapacity = _grow(size + 1);
        T* tmp = _allocate(newCapacity);

        // insert new element
        new (tmp + size) value_type(std::forward<ParamsT>(params)...);

        // move over old elements
        unitialized_move_n(_first, size, tmp);

        // free up old space
        destruct_n(_first, size);
        _deallocate(_first, _sentinel - _first);

        // commit new space
        _first = tmp;
        _last = _first + size + 1;
        _sentinel = _first + newCapacity;

        return _first[size];
    }

    template <typename T>
    template <typename IteratorT, typename SentinelT>
    auto vector<T>::insert(const_iterator pos, IteratorT begin, SentinelT end) -> iterator {
        if constexpr (std::is_same_v<pointer, IteratorT> || std::is_same_v<const_pointer, IteratorT>) {
            UP_SPUD_ASSERT(
                begin < _first || begin >= _last,
                "Inserting a sub-range of a vector into itself is not supported");
        }

        difference_type const count = end - begin;

        if (_sentinel - _last >= count) {
            iterator mpos = _to_iterator(pos);
            if (mpos == _last) {
                unitialized_copy_n(begin, count, mpos);
            }
            else {
                _rshift(mpos, count);
                copy_n(begin, count, mpos);
            }
            _last += count;
            return mpos;
        }

        auto const offset = pos - _first;

        // grow
        auto const newCapacity = _grow(_last - _first + count);
        T* tmp = _allocate(newCapacity);

        // insert new elements
        unitialized_copy_n(begin, count, tmp + offset);

        // move over elements before insertion point
        unitialized_copy_n(_first, offset, tmp);

        // move over elements after insertion point
        unitialized_copy_n(_first + offset, _last - _first - offset, tmp + offset + count);

        auto const new_size = _last - _first + count;

        // free up old space
        destruct_n(_first, _last - _first);
        _deallocate(_first, _sentinel - _first);

        // commit new space
        _first = tmp;
        _last = _first + new_size;
        _sentinel = _first + newCapacity;

        return _first + offset;
    }

    template <typename T>
    auto vector<T>::erase(const_iterator pos) -> iterator {
        iterator mpos = _to_iterator(pos);
        move_n(mpos + 1, _last - mpos - 1, mpos);
        pop_back();
        return mpos;
    }

    template <typename T>
    auto vector<T>::erase(const_iterator begin, const_iterator end) -> iterator {
        iterator mbegin = _to_iterator(begin);
        auto const count = end - begin;
        move_n(mbegin + count, _last - begin - count, mbegin);
        destruct_n(_last - count, count);
        _last -= count;
        return mbegin;
    }

    template <typename T>
    void vector<T>::pop_back() {
        (--_last)->~value_type();
    }

    // note: [[1, 2], 3] will hash the same as [1, [2, 3]]
    //       likewise, ["a", "bc"] will hash the same as ["ab", "c"]

    template <typename HashAlgorithm, typename ValueT>
    inline auto& hash_append(HashAlgorithm& hasher, vector<ValueT> const& container) {
        if constexpr (is_contiguous_v<ValueT>) {
            hasher.append_bytes({container.data(), container.size() * sizeof(ValueT)});
        }
        else {
            for (auto&& value : container) {
                hash_append(hasher, value);
            }
        }
        return hasher;
    }
} // namespace up

#endif
