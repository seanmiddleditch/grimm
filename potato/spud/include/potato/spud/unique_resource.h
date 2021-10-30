// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"

#include <type_traits>
#include <utility>

namespace up {
    template <typename T, auto D>
    class unique_resource;
}

template <typename T, auto D>
class up::unique_resource<T*, D> {
public:
    using pointer = T*;
    using const_pointer = T const*;

    unique_resource() = default;
    ~unique_resource() { reset(); }

    explicit unique_resource(pointer ptr) noexcept : _ptr(ptr) {}

    unique_resource(unique_resource&& src) noexcept : _ptr(src._ptr) { src._ptr = nullptr; }

    unique_resource& operator=(unique_resource&& src) noexcept {
        D(_ptr);
        _ptr = src._ptr;
        src._ptr = nullptr;
        return *this;
    }

    unique_resource& operator=(pointer src) {
        if (_ptr != src) {
            D(_ptr);
            _ptr = src;
        }
        return *this;
    }

    bool empty() const { return _ptr == nullptr; }
    explicit operator bool() const { return _ptr != nullptr; }

    friend bool operator==(unique_resource const& lhs, const_pointer rhs) noexcept { return lhs._ptr == rhs; }

    const_pointer get() const noexcept { return _ptr; }
    pointer get() noexcept { return _ptr; }

    void reset(pointer ptr) {
        if (ptr != _ptr) {
            D(_ptr);
            _ptr = ptr;
        }
    }

    void reset(std::nullptr_t = {}) {
        D(_ptr);
        _ptr = nullptr;
    }

private:
    pointer _ptr = nullptr;
};
