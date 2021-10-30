// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "string_view.h"

#include <cstring>

namespace up {
    template <size_t Capacity>
    class fixed_string;

    template <size_t Capacity>
    fixed_string(char const (&)[Capacity]) -> fixed_string<Capacity>;

    template <size_t Capacity>
    class fixed_string {
        static_assert(Capacity > 0);

    public:
        using value_type = char;
        using pointer = char const*;
        using size_type = size_t;
        static constexpr auto effective_capacity = Capacity - 1;

        constexpr fixed_string() noexcept = default;

        constexpr fixed_string(fixed_string const& string) noexcept;
        constexpr fixed_string(string_view string) noexcept;
        constexpr fixed_string(char const (&string)[Capacity]) noexcept;

        constexpr fixed_string& operator=(fixed_string const& string) noexcept;
        constexpr fixed_string& operator=(string_view string) noexcept;

        constexpr /*implicit*/ operator string_view() const noexcept { return {_buffer, _size}; }

        constexpr explicit operator bool() const noexcept { return _size != 0; }
        [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }

        [[nodiscard]] constexpr size_type size() const noexcept { return _size; }
        [[nodiscard]] constexpr size_type capacity() const noexcept { return effective_capacity; }

        [[nodiscard]] constexpr pointer data() const noexcept { return _buffer; }
        [[nodiscard]] constexpr pointer c_str() const noexcept { return _buffer; }

        constexpr void clear() noexcept;

        template <typename Writer, typename Spec>
        friend void format_value(Writer& writer, fixed_string const& fs, Spec const& options) noexcept {
            format_value_to(writer, string_view{fs._buffer, fs._size}, options);
        }

    private:
        size_t _size = 0;
        char _buffer[Capacity] = {
            '\0',
        };
    };

    template <size_t Capacity>
    constexpr fixed_string<Capacity>::fixed_string(fixed_string const& string) noexcept : _size(string._size) {
        std::memcpy(_buffer, string._buffer, _size + 1);
    }

    template <size_t Capacity>
    constexpr fixed_string<Capacity>::fixed_string(string_view string) noexcept {
        *this = string;
    }

    template <size_t Capacity>
    constexpr fixed_string<Capacity>::fixed_string(char const (&string)[Capacity]) noexcept : _size(Capacity - 1) {
        std::memcpy(_buffer, string, Capacity);
    }

    template <size_t Capacity>
    constexpr auto fixed_string<Capacity>::operator=(fixed_string const& string) noexcept -> fixed_string& {
        if (this != &string) {
            _size = string._size;
            std::memcpy(_buffer, string._buffer, _size + 1);
        }
        return *this;
    }

    template <size_t Capacity>
    constexpr auto fixed_string<Capacity>::operator=(string_view string) noexcept -> fixed_string& {
        size_t newSize = effective_capacity < string.size() ? effective_capacity : string.size();
        _size = newSize;

        if (string.data() >= _buffer && string.data() < _buffer + Capacity) {
            std::memmove(_buffer, string.data(), newSize);
        }
        else {
            std::memcpy(_buffer, string.data(), newSize);
        }
        _buffer[newSize] = '\0';
        return *this;
    }

    template <size_t Capacity>
    constexpr void fixed_string<Capacity>::clear() noexcept {
        _size = 0;
        _buffer[0] = '\0';
    }

    template <typename HashAlgorithm, size_t Size>
    void hash_append(HashAlgorithm& hasher, fixed_string<Size> const& string) {
        hasher(string.data(), string.size());
    }
} // namespace up
