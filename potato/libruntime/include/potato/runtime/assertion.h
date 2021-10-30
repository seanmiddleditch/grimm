// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "debug.h"

#include <nanofmt/format.h>
#include <utility>

#if defined(NDEBUG)

#    define UP_ASSERT(condition, ...) \
        do { } \
        while (false)
#    define UP_UNREACHABLE(...) \
        do { } \
        while (false)

#elif _PREFAST_ // Microsoft's /analyze tool

#    define UP_ASSERT(condition, ...) __analysis_assume(condition)
#    define UP_UNREACHABLE(...) __analysis_assume(false)

#else

namespace up::_detail {
    // abstraction to deal with assert instances that don't have a message at all
    template <int N, typename... Args>
    void constexpr formatAssertion(char (&buffer)[N], nanofmt::format_string format, Args&&... args) {
        nanofmt::format_to(buffer, format, std::forward<Args>(args)...);
    }
    template <typename DestT>
    void constexpr formatAssertion(DestT& dest) { }
} // namespace up::_detail

#    define uppriv_FORMAT_FAIL(condition_text, ...) \
        do { \
            char uppriv_fail_buffer[512] = { \
                0, \
            }; \
            nanofmt::format_to(uppriv_fail_buffer, (condition_text), ##__VA_ARGS__); \
            uppriv_FAIL((condition_text), uppriv_fail_buffer); \
        } \
        while (false)

#    define UP_ASSERT(condition, ...) \
        do { \
            if (UP_UNLIKELY(!((condition)))) { \
                uppriv_FORMAT_FAIL(#condition, ##__VA_ARGS__); \
            } \
        } \
        while (false)

#    define UP_UNREACHABLE(...) uppriv_FAIL("unreachable code", ##__VA_ARGS__)

#endif // _PREFAST_
