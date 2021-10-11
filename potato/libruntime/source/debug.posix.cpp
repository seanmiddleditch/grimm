// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "debug.h"

#include <cstdio>
#include <cstdlib>

namespace up::_detail {
    auto handleFatalError(
        char const* file,
        int line,
        char const* failedConditionText,
        char const* messageText,
        char const* callstackText) -> FatalErrorAction {
        std::fprintf(
            stderr,
            "%s(%d): failed %s: %s\n%s\n",
            file,
            line,
            failedConditionText,
            messageText,
            callstackText);
        std::abort();
    }
} // namespace up::_detail
