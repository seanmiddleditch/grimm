// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/platform.h"

#include <cstdlib>

/// Break the debugger, if attached, at this point.
#if defined(UP_COMPILER_MICROSOFT)
#    define UP_DEBUG_BREAK() __debugbreak()
#elif defined(UP_PLATFORM_POSIX)
#    include <signal.h> // NOLINT(modernize-deprecated-headers)
#    define UP_DEBUG_BREAK() \
        do { \
            raise(SIGTRAP); \
        } while (false)
#else
#    error "UP_DEBUG_BREAK: Unsupported platform/compiler"
#endif

namespace up::_detail {
    /// Results from error_dialog.
    enum class FatalErrorAction {
        Abort,
        BreakInDebugger,
        IgnoreOnce,
        IgnoreAlways,
    };

    /// Ask the user if they should abort, skip the error, or always skip this error.
    UP_RUNTIME_API UP_NOINLINE FatalErrorAction
    raiseFatalError(char const* file, int line, char const* failedConditionText, char const* messageText);
} // namespace up::_detail

#define uppriv_FAIL(failure, message) \
    do { \
        if (static bool uppriv_IgnoreFailure = false; UP_LIKELY(!uppriv_IgnoreFailure)) { \
            switch (::up::_detail::raiseFatalError(__FILE__, __LINE__, (failure), (message))) { \
                case ::up::_detail::FatalErrorAction::Abort: \
                    std::abort(); \
                    break; \
                case ::up::_detail::FatalErrorAction::BreakInDebugger: \
                    UP_DEBUG_BREAK(); \
                    break; \
                case ::up::_detail::FatalErrorAction::IgnoreOnce: \
                    break; \
                case ::up::_detail::FatalErrorAction::IgnoreAlways: \
                    uppriv_IgnoreFailure = true; \
                    break; \
            } \
        } \
    } while (false)
