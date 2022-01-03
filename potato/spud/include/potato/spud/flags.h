// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "traits.h"

namespace up::flag_enums { } // namespace up::flag_enums

#define UP_DEFINE_FLAGS(NAME, UNDERLYING, ...) \
    namespace ft_##NAME { \
        enum class NAME : UNDERLYING { __VA_ARGS__ }; \
        constexpr NAME operator|(NAME lhs, NAME rhs) noexcept { \
            return static_cast<NAME>(static_cast<UNDERLYING>(lhs) | static_cast<UNDERLYING>(rhs)); \
        } \
        constexpr NAME operator&(NAME lhs, NAME rhs) noexcept { \
            return static_cast<NAME>(static_cast<UNDERLYING>(lhs) & static_cast<UNDERLYING>(rhs)); \
        } \
        constexpr NAME operator~(NAME val) noexcept { return static_cast<NAME>(~static_cast<UNDERLYING>(val)); } \
    } \
    using ft_##NAME::NAME
