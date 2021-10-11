// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/fixed_string.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"

namespace up::callstack {
    struct TraceRecord {
        static constexpr int symbol_length = 128;

        fixed_string<symbol_length> filename;
        fixed_string<symbol_length> symbol;
        uintptr address = 0;
        int line = 0;
    };

    [[nodiscard]] UP_RUNTIME_API auto readTrace(span<uintptr> addresses, uint skip = 0) -> span<uintptr>;
    [[nodiscard]] UP_RUNTIME_API auto resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records)
        -> span<TraceRecord>;
} // namespace up::callstack
