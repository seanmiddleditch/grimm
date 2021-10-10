// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_assertion.h"

#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <nanofmt/format.h>
#include <cstring>

namespace up {
    class string;

    class string_writer {
    public:
        using value_type = char;
        using iterator = value_type*;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using const_iterator = value_type const*;
        using size_type = std::size_t;

        string_writer() = default;
        ~string_writer() { reset(); }

        explicit string_writer(size_type capacity) {
            reserve(capacity);
            clear();
        }

        string_writer(string_writer const&) = delete;
        string_writer& operator=(string_writer const&) = delete;

        bool empty() const noexcept { return _size == 0; }
        explicit operator bool() const noexcept { return _size != 0; }

        size_type size() const noexcept { return _size; }
        size_type capacity() const { return _capacity - 1 /*NUL byte*/; }
        const_pointer data() const noexcept { return _ptr; }

        const_pointer c_str() const noexcept {
            UP_SPUD_ASSERT(_ptr[_size] == '\0', "acquire() operation did not commit()");
            return _ptr;
        }

        /*implicit*/ operator string_view() const noexcept { return {_ptr, _size}; }
        /*implicit*/ operator zstring_view() const noexcept { return c_str(); }

        inline void append(value_type ch);
        inline void append(const_pointer data, size_type length);
        void append(string_view str) { append(str.data(), str.size()); }

        template <typename... Args>
        void format(nanofmt::format_string format, Args const&... args);
        inline void vformat(nanofmt::format_string format, nanofmt::format_args&& args);

        // for back_inserter support
        void push_back(value_type ch) { append(ch); }

        inline void reserve(size_type capacity);

        [[nodiscard]] inline span<value_type> acquire(size_type size);
        inline void commit(span<value_type const> data);

        inline void resize(size_type newSize, value_type fill = ' ');

        void clear() {
            *_ptr = 0;
            _size = 0;
        }

        inline void reset();

        inline string to_string() const&;
        inline string to_string() &&;

    private:
        inline void _grow(size_type requiredSize);

        static constexpr size_type fixed_size = 512;

        size_type _size = 0;
        size_type _capacity = fixed_size;
        pointer _ptr = _fixed;
        value_type _fixed[fixed_size] = {
            0,
        };
    };
} // namespace up

void up::string_writer::append(value_type ch) {
    _grow(_size + 1);
    _ptr[_size++] = ch;
    _ptr[_size] = '\0';
}

void up::string_writer::append(const_pointer data, size_type length) {
    _grow(_size + length);
    std::memmove(_ptr + _size, data, length);
    _size += length;
    _ptr[_size] = '\0';
}

template <typename... Args>
void up::string_writer::format(nanofmt::format_string format, Args const&... args) {
    return vformat(format, nanofmt::make_format_args(args...));
}

void up::string_writer::vformat(nanofmt::format_string format, nanofmt::format_args&& args) {
    char buffer[1024] = {
        '\0',
    };
    char const* const end = nanofmt::format_to(buffer, format, std::move(args));
    append(buffer, end - buffer);
}

void up::string_writer::reserve(size_type capacity) {
    if (capacity >= _capacity) {
        size_type newCapacity = capacity + 1;
        UP_SPUD_ASSERT(newCapacity > capacity, "overflow");

        auto newBuffer = new value_type[newCapacity];

        std::memcpy(newBuffer, _ptr, _size + 1 /*NUL*/);

        if (_ptr != static_cast<char*>(_fixed)) {
            delete[] _ptr;
        }

        _ptr = newBuffer;
        _capacity = newCapacity;
    }
}

auto up::string_writer::acquire(size_type size) -> span<char> {
    _grow(_size + size);
    return {_ptr + _size, _capacity - 1 - _size};
}

void up::string_writer::commit(span<char const> data) {
    UP_SPUD_ASSERT(data.data() == _ptr + _size, "commit() does not match acquire()d buffer");
    UP_SPUD_ASSERT(data.size() <= _capacity - 1 - _size, "commit() size exceeds acquired()d buffer");

    _size += data.size();
    _ptr[_size] = '\0';
}

void up::string_writer::reset() {
    if (_ptr != static_cast<char*>(_fixed)) {
        delete[] _ptr;
        _ptr = _fixed;
        _capacity = sizeof(_fixed);
    }
    _size = 0;
    *_ptr = '\0';
}

auto up::string_writer::to_string() const& -> string {
    return string(_ptr, _size);
}

auto up::string_writer::to_string() && -> string {
    string result;

    if (_ptr != _fixed && _size == _capacity - 1 /*NUL*/) {
        result = string::take_ownership(_ptr, _size);
        _ptr = nullptr;
    }
    else {
        result = string(_ptr, _size);
    }

    reset();
    return result;
}

void up::string_writer::resize(size_type newSize, value_type fill) {
    if (_size < newSize) {
        _grow(newSize);
        std::memset(_ptr + _size, fill, newSize - _size);
        _size = newSize;
    }
    _size = newSize;
    _ptr[_size] = 0;
}

void up::string_writer::_grow(size_type requiredSize) {
    // >= to account for NUL byte
    if (requiredSize >= _capacity) {
        size_type newCapacity = _capacity * 2;
        if (newCapacity <= requiredSize) {
            newCapacity = requiredSize + 1 /*NUL*/;
        }

        auto newBuffer = new value_type[newCapacity];

        std::memcpy(newBuffer, _ptr, _size + 1 /*NUL*/);

        if (_ptr != static_cast<char*>(_fixed)) {
            delete[] _ptr;
        }

        _ptr = newBuffer;
        _capacity = newCapacity;
    }
}
