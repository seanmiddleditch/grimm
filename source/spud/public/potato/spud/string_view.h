// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "numeric_util.h"
#include "string_util.h"

#include <nanofmt/forward.h>
#include <compare>
#include <type_traits>
#include <utility>

namespace up {
    class string_view;

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, string_view const& string);

    constexpr string_view operator"" _sv(char const* str, size_t size) noexcept;

    template <typename T>
    concept has_c_str = std::is_convertible_v<decltype(std::declval<T>().c_str()), char const*>;
} // namespace up

class up::string_view {
public:
    using value_type = char;
    using iterator = char const*;
    using const_iterator = char const*;
    using pointer = char const*;
    using reference = char const&;
    using size_type = size_t;

    static constexpr size_type npos = ~size_type{0};

    constexpr string_view() noexcept = default;

    constexpr string_view(string_view const&) noexcept = default;
    constexpr string_view(string_view&&) noexcept = default;

    /*implicit*/ constexpr string_view(pointer zstr) noexcept
        : _data(zstr)
        , _size(zstr != nullptr ? stringLength(zstr) : 0) {}
    /*implicit*/ constexpr string_view(pointer data, size_type size) noexcept : _data(data), _size(size) {}
    /*implicit*/ constexpr string_view(pointer begin, pointer end) noexcept : _data(begin), _size(end - begin) {}
    template <typename StringT>
    /*implicit*/ constexpr string_view(StringT const& str) noexcept requires has_c_str<StringT>
        : _data(str.c_str())
        , _size(str.size()) {}

    constexpr string_view& operator=(string_view const&) noexcept = default;
    constexpr string_view& operator=(string_view&&) noexcept = default;

    constexpr string_view& operator=(pointer zstr) noexcept {
        _data = zstr;
        _size = zstr != nullptr ? stringLength(zstr) : 0;
        return *this;
    }

    constexpr pointer data() const noexcept { return _data; }
    constexpr size_type size() const noexcept { return _size; }

    constexpr bool empty() const noexcept { return _size == 0; }
    constexpr explicit operator bool() const noexcept { return _size != 0; }

    constexpr const_iterator begin() const noexcept { return _data; }
    constexpr const_iterator end() const noexcept { return _data + _size; }

    constexpr value_type front() const noexcept { return *_data; }
    constexpr value_type back() const noexcept { return *(_data + _size - 1); }

    constexpr value_type operator[](size_type index) const noexcept { return _data[index]; }

    constexpr void pop_front() noexcept {
        ++_data;
        --_size;
    }
    constexpr void pop_back() noexcept { --_size; }

    constexpr void remove_prefix(size_type count) noexcept {
        _data += count;
        _size -= count;
    }
    constexpr void remove_suffix(size_type count) noexcept { _size -= count; }

    constexpr string_view first(size_type count) const noexcept { return {_data, count}; }
    constexpr string_view last(size_type count) const noexcept { return {_data + _size - count, count}; }

    constexpr string_view substr(size_type offset, size_type count = npos) const noexcept {
        if (offset > _size) {
            offset = _size;
        }
        auto const remaining = _size - offset;
        if (count > remaining) {
            count = remaining;
        }
        return {_data + offset, count};
    }

    constexpr bool starts_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data, str.data(), str.size()) == 0;
    }
    constexpr bool ends_with(string_view str) const noexcept {
        if (str.size() > _size) {
            return false;
        }
        return stringCompare(_data + _size - str.size(), str.data(), str.size()) == 0;
    }

    constexpr size_type find(value_type ch) const noexcept {
        auto iter = stringFindChar(_data, _size, ch);
        return iter != nullptr ? iter - _data : npos;
    }

    constexpr size_type find_first_of(string_view chars) const noexcept {
        for (size_type i = 0; i != _size; ++i) {
            if (stringFindChar(chars._data, chars._size, _data[i]) != nullptr) {
                return i;
            }
        }
        return npos;
    }

    constexpr size_type find_last_of(string_view chars) const noexcept {
        for (size_type i = _size; i != 0; --i) {
            if (stringFindChar(chars._data, chars._size, _data[i - 1]) != nullptr) {
                return i - 1;
            }
        }
        return npos;
    }

    friend bool constexpr operator==(string_view lhs, string_view rhs) noexcept {
        return lhs.size() == rhs.size() && stringCompare(lhs.data(), rhs.data(), lhs.size()) == 0;
    }

    friend std::strong_ordering constexpr operator<=>(string_view lhs, string_view rhs) noexcept {
        auto const len = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        auto const rs = stringCompare(lhs.data(), rhs.data(), len);
        return rs <=> 0;
    }

    template <typename Output>
    friend auto operator<<(Output& output, string_view str) -> Output& {
        return output.write(str.data(), str.size());
    }

private:
    pointer _data = nullptr;
    size_type _size = 0;
};

template <typename HashAlgorithm>
void up::hash_append(HashAlgorithm& hasher, string_view const& string) {
    hasher.append_bytes(string.data(), string.size());
}

constexpr auto up::operator"" _sv(char const* str, size_t size) noexcept -> string_view {
    return {str, size};
}

namespace nanofmt {
    template <>
    struct formatter<up::string_view> {
        constexpr char const* parse(char const* in, char const*) noexcept { return in; }

        template <typename OutputT>
        void format(up::string_view str, OutputT& output) noexcept {
            output.append(str.data(), str.size());
        }
    };
} // namespace nanofmt
