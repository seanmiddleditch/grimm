// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>
#include <utility>

namespace up {

    template <typename T>
    class com_ptr {
    public:
        using pointer = T*;
        using reference = T&;

        com_ptr() = default;
        ~com_ptr() { _decRef(); }

        explicit com_ptr(T* ptr) noexcept : _ptr(ptr) {}
        com_ptr(std::nullptr_t) noexcept {}

        com_ptr(com_ptr const& rhs) : _ptr(rhs._ptr) { _addRef(); }
        com_ptr(com_ptr&& rhs) noexcept : _ptr(rhs._ptr) { rhs._ptr = nullptr; }

        template <typename To>
        com_ptr<To> as() const noexcept {
            To* ptr = static_cast<To*>(_ptr);
            if (_ptr != nullptr) {
                _addRef();
            }
            return com_ptr<To>(ptr);
        }

        inline com_ptr& operator=(com_ptr const& rhs);
        inline com_ptr& operator=(com_ptr&& rhs) noexcept;
        inline com_ptr& operator=(std::nullptr_t) noexcept;

        explicit operator bool() const noexcept { return _ptr != nullptr; }
        bool empty() const noexcept { return _ptr == nullptr; }

        inline void reset(pointer ptr = pointer{});

        [[nodiscard]] inline pointer release() noexcept;

        pointer get() const noexcept { return _ptr; }
        pointer operator->() const noexcept { return _ptr; }
        reference operator*() const noexcept { return *_ptr; }

        friend bool operator==(com_ptr const& lhs, com_ptr const& rhs) noexcept { return lhs.get() == rhs.get(); }
        friend bool operator!=(com_ptr const& lhs, com_ptr const& rhs) noexcept { return lhs.get() != rhs.get(); }
        friend bool operator==(com_ptr const& lhs, std::nullptr_t) noexcept { return lhs.get() == nullptr; }
        friend bool operator!=(com_ptr const& lhs, std::nullptr_t) noexcept { return lhs.get() != nullptr; }
        friend bool operator==(std::nullptr_t, com_ptr const& rhs) noexcept { return nullptr == rhs.get(); }
        friend bool operator!=(std::nullptr_t, com_ptr const& rhs) noexcept { return nullptr != rhs.get(); }

    private:
        void _addRef() const {
            if (_ptr != nullptr) {
                _ptr->AddRef();
            }
        }
        void _decRef() const {
            if (_ptr != nullptr) {
                _ptr->Release();
            }
        }

        pointer _ptr = nullptr;
    };

    template <typename T>
    auto com_ptr<T>::operator=(com_ptr const& rhs) -> com_ptr& {
        if (this != std::addressof(rhs)) {
            _decRef();
            _ptr = rhs._ptr;
            _addRef();
        }
        return *this;
    }

    template <typename T>
    auto com_ptr<T>::operator=(com_ptr&& rhs) noexcept -> com_ptr& {
        if (this != std::addressof(rhs)) {
            _decRef();
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
        }
        return *this;
    }

    template <typename T>
    auto com_ptr<T>::operator=(std::nullptr_t) noexcept -> com_ptr& {
        _decRef();
        _ptr = nullptr;
        return *this;
    }

    template <typename T>
    void com_ptr<T>::reset(pointer ptr) {
        if (ptr != _ptr) {
            _decRef();
            _ptr = ptr;
        }
    }

    template <typename T>
    auto com_ptr<T>::release() noexcept -> pointer {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

} // namespace up
