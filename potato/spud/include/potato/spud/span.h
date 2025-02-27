// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"

#include <cstddef>

namespace up {
    template <typename T>
    class span;

    template <typename T>
    using view = span<T const>;

    template <typename PointerT, typename ValueT = decltype(*declval<PointerT>())>
    class deref_span;

    template <typename T>
    span(T*, T*) -> span<T>;
    template <typename T>
    span(T*, std::size_t) -> span<T>;
    template <typename T, std::size_t N>
    span(T (&src)[N]) -> span<T>;

    template <typename T>
    class vector;

    template <typename T>
    span(vector<T> const&) -> span<T const>;
    template <typename T>
    span(vector<T>&) -> span<T>;

    template <typename HashAlgorithm, typename T>
    inline void hash_append(HashAlgorithm&, up::span<T> const&) noexcept;
} // namespace up

/// <summary> A non-owning slice of an array. </summary>
/// <typeparam name="T"> Type of the elements in the array. </typeparam>
template <typename T>
class up::span {
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = T const*;
    using sentinel = T*;
    using pointer = T*;
    using reference = T&;
    using size_type = std::size_t;
    using index_type = size_type;

    constexpr span() noexcept = default;
    constexpr span(span const&) noexcept = default;
    constexpr span(span&&) noexcept = default;
    template <typename U>
    /*implicit*/ constexpr span(span<U> src) noexcept : _begin(src.begin())
                                                      , _end(src.end()) { }
    /*implicit*/ constexpr span(T* begin, T* end) noexcept : _begin(begin), _end(end) { }
    /*implicit*/ constexpr span(T* ptr, std::size_t size) noexcept : _begin(ptr), _end(ptr + size) { }
    template <std::size_t N>
    /*implicit*/ constexpr span(T (&src)[N]) noexcept : _begin(src)
                                                      , _end(src + N) { }

    constexpr span& operator=(span&&) noexcept = default;

    constexpr iterator begin() const noexcept { return _begin; }
    constexpr sentinel end() const noexcept { return _end; }

    constexpr pointer data() const noexcept { return _begin; }

    constexpr bool empty() const noexcept { return _begin == _end; }
    constexpr explicit operator bool() const noexcept { return _begin != _end; }

    constexpr reference operator[](size_type index) const noexcept { return _begin[index]; }

    constexpr size_type size() const noexcept { return static_cast<size_type>(_end - _begin); }

    constexpr reference front() const noexcept { return *_begin; }
    constexpr reference back() const noexcept { return *(_end - 1); }

    constexpr span first(size_type length) const noexcept { return span{_begin, length}; };
    constexpr span last(size_type length) const noexcept { return span{_end - length, length}; };

    constexpr span subspan(size_type offset, size_type count) const noexcept { return span{_begin + offset, count}; }
    constexpr span subspan(size_type offset) const noexcept { return span{_begin + offset, (_end - _begin) - offset}; }

    auto as_bytes() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return span<std::byte const>{
                reinterpret_cast<std::byte const*>(_begin),
                static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
        else {
            return span<std::byte>{
                reinterpret_cast<std::byte*>(_begin),
                static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
    }

    auto as_chars() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return span<char const>{
                reinterpret_cast<char const*>(_begin),
                static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
        else {
            return span<char>{reinterpret_cast<char*>(_begin), static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
    }

    constexpr void pop_front() noexcept { ++_begin; }
    constexpr void pop_back() noexcept { --_end; }

private:
    pointer _begin = nullptr;
    pointer _end = nullptr;
};

template <typename PointerT, typename ValueT>
class up::deref_span {
public:
    using value_type = ValueT;
    using size_type = size_t;
    using reference = ValueT&;

    class iterator {
    public:
        constexpr explicit iterator(PointerT* ptr) noexcept : _ptr(ptr) { }

        constexpr reference operator*() const noexcept { return **_ptr; }
        constexpr iterator& operator++() noexcept {
            ++_ptr;
            return *this;
        }

        constexpr bool operator==(iterator it) const noexcept { return _ptr == it._ptr; }

    private:
        PointerT* _ptr = nullptr;
    };

    template <typename RangeT>
    constexpr deref_span(RangeT const& range) noexcept : _first(range.data())
                                                       , _last(_first + range.size()) { }

    constexpr iterator begin() const noexcept { return iterator(_first); }
    constexpr iterator end() const noexcept { return iterator(_last); }

    constexpr size_type size() const noexcept { return _last - _first; }

private:
    PointerT* _first = nullptr;
    PointerT* _last = nullptr;
};

template <typename HashAlgorithm, typename T>
void up::hash_append(HashAlgorithm& hasher, up::span<T> const& view) noexcept {
    if constexpr (up::is_contiguous_v<T>) {
        hasher.append_bytes(reinterpret_cast<char const*>(view.data()), view.size() * sizeof(T));
    }
    else {
        for (auto&& value : view) {
            hash_append(hasher, value);
        }
    }
}
