// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/int_types.h"
#include "potato/spud/zstring_view.h"

namespace up {

    using SmallThreadId = uint16;

    UP_RUNTIME_API SmallThreadId currentSmallThreadId() noexcept;

    UP_RUNTIME_API void setCurrentThreadName(zstring_view name) noexcept;

} // namespace up
