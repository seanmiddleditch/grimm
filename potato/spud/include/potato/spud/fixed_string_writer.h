// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "string_view.h"

#include <cstring>

namespace up {
    template <std::size_t Capacity>
    class fixed_string_writer {
    public:
        using value_type = char;
        using iterator = value_type*;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using const_iterator = value_type const*;
        using size_type = std::size_t;

        fixed_string_writer() = default;
        ~fixed_string_writer() = default;

        fixed_string_writer(fixed_string_writer const&) = delete;
        fixed_string_writer& operator=(fixed_string_writer const&) = delete;

        bool empty() const noexcept { return _size == 0; }
        explicit operator bool() const noexcept { return _size != 0; }

        size_type size() const noexcept { return _size; }
        size_type capacity() const noexcept { return Capacity - 1 /*NUL byte*/; }
        const_pointer data() const noexcept { return _buffer; }

        const_pointer c_str() const noexcept { return _buffer; }

        /*implicit*/ operator string_view() const noexcept { return {_buffer, _size}; }

        void append(const_pointer nstr, size_type len) noexcept {
            size_type available = capacity() - _size;
            size_type writeLen = available < len ? available : len;
            std::memmove(_buffer + _size, nstr, writeLen);
            _buffer[_size += writeLen] = 0;
        }

        void append(string_view str) noexcept { append(str.data(), str.size()); }

        void append(value_type ch) noexcept {
            if (_size < capacity()) {
                _buffer[_size] = ch;
                _buffer[++_size] = 0;
            }
        }

        // for back_inserter support
        void push_back(value_type ch) noexcept { append(ch); }

        void clear() noexcept {
            *_buffer = 0;
            _size = 0;
        }

    private:
        size_type _size = 0;
        value_type _buffer[Capacity] = {
            0,
        };
    };
} // namespace up
