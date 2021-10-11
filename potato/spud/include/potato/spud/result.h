// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"

namespace up {
    template <typename ValueT, enumeration StatusT, StatusT Success = StatusT{}>
    struct Result {
        StatusT status = StatusT{};
        ValueT value = ValueT{};

        constexpr explicit operator bool() const noexcept { return status == StatusT{}; }
    };
} // namespace up
