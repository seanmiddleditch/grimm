// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <grimm/foundation/int_types.h>
#include <grimm/foundation/zstring_view.h>

namespace gm::concurrency {

    using SmallThreadId = uint16;

    GM_CONCURRENCY_API SmallThreadId currentSmallThreadId() noexcept;

    GM_CONCURRENCY_API void setCurrentThreadName(zstring_view name);

} // namespace gm::concurrency
