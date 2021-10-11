// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/result.h"

namespace up {
    enum class IOResult {
        Success,
        AccessDenied,
        FileNotFound,
        System,
        InvalidArgument,
        UnsupportedOperation,
        Malformed,
        Unknown,
    };

    template <typename T>
    using IOReturn = Result<T, IOResult>;
} // namespace up
