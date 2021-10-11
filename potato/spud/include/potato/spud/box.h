// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"

#include <compare>
#include <utility>

namespace up {
    template <typename>
    class box;
    template <typename T, typename... Args>
    auto new_box(Args&&... args) -> box<T> requires std::is_constructible_v<T, Args&&...>;

    namespace _detail {
        template <typename>
        struct box_traits;
    }
} // namespace up

template <typename T>
struct up::_detail::box_traits {
    using pointer = T*;
    using reference = T&;

    static void _deallocate(T* ptr) { delete ptr; }
};

/// <summary> An owning non-copyable pointer to a heap-allocated object, analogous to std::unique_ptr but without custom
/// deleter support. </summary> <typeparam name="T"> Type of the object owned by the box. </typeparam>
template <typename T>
class up::box : private up::_detail::box_traits<T> {
public:
    using value_type = T;
    using pointer = typename up::_detail::box_traits<T>::pointer;
    using reference = typename up::_detail::box_traits<T>::reference;

    box() noexcept = default;
    ~box() { reset(); }

    box(std::nullptr_t) noexcept {}
    box& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    explicit box(pointer ptr) noexcept : _ptr(ptr) {}

    box(box&& src) noexcept : _ptr(src.release()) {}
    box& operator=(box&& src) noexcept {
        reset(src.release());
        return *this;
    }

    template <typename U>
    box(box<U>&& src) noexcept requires std::is_convertible_v<U*, T*> : _ptr(src.release()) {}
    template <typename U>
    auto operator=(box<U>&& src) noexcept -> box<T>& requires std::is_convertible_v<U*, T*> {
        reset(src.release());
        return *this;
    }

    pointer get() const noexcept { return _ptr; }
    pointer operator->() const noexcept { return _ptr; }

    reference operator*() const noexcept { return *_ptr; }

    explicit operator bool() const noexcept { return _ptr != nullptr; }
    bool empty() const noexcept { return _ptr == nullptr; }

    inline void reset(pointer ptr = pointer{}) noexcept;
    [[nodiscard]] pointer release() noexcept {
        pointer tmp = _ptr;
        _ptr = nullptr;
        return tmp;
    }

    template <typename U>
    friend class box;

    template <typename U>
    friend auto operator<=>(box const& lhs, box<U> const& rhs) noexcept {
        return lhs._ptr <=> rhs._ptr;
    }
    template <typename U>
    friend bool operator==(box const& lhs, box<U> const& rhs) noexcept {
        return lhs._ptr == rhs._ptr;
    }

    template <typename U>
    friend bool operator<=>(box const& lhs, U const* rhs) noexcept {
        return lhs._ptr <=> rhs;
    }
    template <typename U>
    friend bool operator==(box const& lhs, U const* rhs) noexcept {
        return lhs._ptr == rhs;
    }

    template <typename U>
    friend bool operator<=>(box const& lhs, std::nullptr_t rhs) noexcept {
        return lhs._ptr <=> rhs;
    }
    template <typename U>
    friend bool operator==(box const& lhs, std::nullptr_t* rhs) noexcept {
        return lhs._ptr == rhs;
    }

private:
    pointer _ptr = nullptr;
};

template <typename T>
void up::box<T>::reset(pointer ptr) noexcept {
    // NOLINTNEXTLINE(bugprone-sizeof-expression)
    static_assert(sizeof(T) > 0, "box can not delete incomplete type");
    this->_deallocate(_ptr);
    _ptr = ptr;
}

/// <summary> Box an object; allocates a new instance and returns a box that owns the allocation </summary>
/// <typeparam name="T"> Type of object to box. </typeparam>
/// <param name="args"> Parameters to pass to the constructor. </param>
/// <returns> A box containing a new instance of the requested object. </returns>
template <typename T, typename... Args>
auto up::new_box(Args&&... args) -> box<T> requires std::is_constructible_v<T, Args&&...> {
    return box<T>(new T(std::forward<Args>(args)...));
}
