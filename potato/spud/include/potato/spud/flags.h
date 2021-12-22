// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "traits.h"

namespace up::flags_ops {
    template <enumeration E, typename U = std::underlying_type_t<E>>
    constexpr E operator|(E lhs, E rhs) noexcept {
        return static_cast<E>(static_cast<U>(lhs) | static_cast<U>(rhs));
    }

    template <enumeration E, typename U = std::underlying_type_t<E>>
    constexpr E& operator|=(E& lhs, E rhs) noexcept {
        return static_cast<E&>(static_cast<U&>(lhs) |= static_cast<U>(rhs));
    }

    template <enumeration E, typename U = std::underlying_type_t<E>>
    constexpr E operator&(E lhs, E rhs) noexcept {
        return static_cast<E>(static_cast<U>(lhs) & static_cast<U>(rhs));
    }

    template <enumeration E, typename U = std::underlying_type_t<E>>
    constexpr E operator&=(E& lhs, E rhs) noexcept {
        return static_cast<E&>(static_cast<U&>(lhs) &= static_cast<U>(rhs));
    }

    template <enumeration E, typename U = std::underlying_type_t<E>>
    constexpr E operator~(E val) noexcept {
        return static_cast<E>(~static_cast<U>(val));
    }
} // namespace up::flags_ops

#define UP_DEFINE_FLAGS(name, underlying, ...) \
    namespace ft_##name { \
        using namespace ::up::flags_ops; \
        enum class name : underlying { __VA_ARGS__ }; \
    } \
    using ft_##name::name
