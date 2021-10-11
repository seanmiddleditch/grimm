// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if !defined(DOXYGEN_SHOULD_SKIP_THIS) // Doxygen can't handle our use of C++20 concepts currently

#    include <atomic>
#    include <utility>

namespace up {
    template <typename T>
    class rc;

    constexpr struct {
    } rc_acquire;

    template <typename Derived>
    class shared {
    public:
        shared(shared const&) = delete;
        shared& operator=(shared const&) = delete;

        // moving is allowed, but does not impact references
        shared(shared&&) noexcept {}
        shared& operator=(shared&&) noexcept { return *this; }

    protected:
        shared() noexcept = default;

        // explicitly not virtual, since nobody can delete via shared<T>
        ~shared() = default;

    private:
        template <typename U>
        friend class rc;

        void addRef() const noexcept { ++_refs; }
        void removeRef() const noexcept {
            if (--_refs == 0) {
                delete static_cast<Derived const*>(this);
            }
        }

        mutable std::atomic<int> _refs = 1;
    };

    template <typename T>
    class rc {
    public:
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        rc() noexcept = default;
        explicit rc(pointer ptr) noexcept : _ptr(ptr) {}
        rc(decltype(rc_acquire), pointer ptr) noexcept : _ptr(ptr) { _addRef(); }
        ~rc() noexcept { _removeRef(); }

        rc(rc const& rhs) noexcept : _ptr(rhs._ptr) { _addRef(); }
        rc(rc&& rhs) noexcept : _ptr(rhs.release()) {}
        template <typename U>
        /*implicit*/ rc(rc<U> const& rhs) noexcept requires std::is_convertible_v<U*, T*> : _ptr(rhs.get()) {
            _addRef();
        }
        template <typename U>
        /*implicit*/ rc(rc<U>&& rhs) noexcept requires std::is_convertible_v<U*, T*> : _ptr(rhs.release()) {}
        /*implicit*/ rc(std::nullptr_t) noexcept {}

        inline rc& operator=(rc const& rhs) noexcept;
        inline rc& operator=(rc&& rhs) noexcept;
        template <typename U>
        inline rc& operator=(rc<U> const& rhs) noexcept requires std::is_convertible_v<U*, T*>;
        template <typename U>
        inline rc& operator=(rc<U>&& rhs) noexcept requires std::is_convertible_v<U*, T*>;
        inline rc& operator=(std::nullptr_t) noexcept;

        explicit operator bool() const noexcept { return _ptr != nullptr; }
        bool empty() const noexcept { return _ptr == nullptr; }

        inline void reset(pointer ptr = pointer{}) noexcept;

        [[nodiscard]] inline pointer release() noexcept;

        pointer get() const noexcept { return _ptr; }
        pointer operator->() const noexcept { return _ptr; }
        reference operator*() const noexcept { return *_ptr; }

        friend auto operator<=>(rc const& lhs, rc const& rhs) noexcept { return lhs.get() <=> rhs.get(); }
        friend bool operator==(rc const& lhs, rc const& rhs) noexcept { return lhs.get() == rhs.get(); }

        template <typename U>
        friend auto operator<=>(rc const& lhs, rc<U> const& rhs) noexcept {
            return lhs.get() <=> rhs.get();
        }
        template <typename U>
        friend bool operator==(rc const& lhs, rc<U> const& rhs) noexcept {
            return lhs.get() == rhs.get();
        }

        friend auto operator<=>(rc const& lhs, std::nullptr_t rhs) noexcept { return lhs.get() <=> rhs; }
        friend bool operator==(rc const& lhs, std::nullptr_t rhs) noexcept { return lhs.get() == rhs; }

    private:
        void _addRef() noexcept {
            if (_ptr != nullptr) {
                _ptr->addRef();
            }
        }
        void _removeRef() noexcept {
            if (_ptr != nullptr) {
                _ptr->removeRef();
            }
        }

        pointer _ptr = nullptr;
    };

    template <typename T>
    auto rc<T>::operator=(rc const& rhs) noexcept -> rc& {
        if (this != &rhs) {
            _removeRef();
            _ptr = rhs._ptr;
            _addRef();
        }
        return *this;
    }

    template <typename T>
    template <typename U>
    auto rc<T>::operator=(rc<U> const& rhs) noexcept -> rc& requires std::is_convertible_v<U*, T*> {
        if (this != &rhs) {
            _removeRef();
            _ptr = rhs.get();
            _addRef();
        }
        return *this;
    }

    template <typename T>
    auto rc<T>::operator=(rc&& rhs) noexcept -> rc& {
        if (this != &rhs) {
            _removeRef();
            _ptr = rhs.release();
        }
        return *this;
    }

    template <typename T>
    template <typename U>
    auto rc<T>::operator=(rc<U>&& rhs) noexcept -> rc& requires std::is_convertible_v<U*, T*> {
        if (this != &rhs) {
            _removeRef();
            _ptr = rhs.release();
        }
        return *this;
    }

    template <typename T>
    auto rc<T>::operator=(std::nullptr_t) noexcept -> rc& {
        _removeRef();
        _ptr = nullptr;
        return *this;
    }

    template <typename T>
    void rc<T>::reset(pointer ptr) noexcept {
        if (ptr != _ptr) {
            _removeRef();
            _ptr = ptr;
        }
    }

    template <typename T>
    auto rc<T>::release() noexcept -> pointer {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

    template <typename T, typename... A>
    rc<T> new_shared(A&&... args) {
        return rc<T>(new T(std::forward<A>(args)...));
    }

} // namespace up

#endif
