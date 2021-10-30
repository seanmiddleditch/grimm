// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "traits.h"
#include "utility.h"

namespace up {
    template <range Range, equality_comparable_with<range_value_t<Range>> T>
    constexpr auto erase(Range& range, T const& value) -> typename Range::size_type {
        auto iter = begin(range);
        auto out = iter;
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (*iter != value) {
                *out++ = std::move(*iter);
            }
        }

        auto const erased = last - out;
        range.erase(out, last);
        return erased;
    }

    template <range Range, typename T, projection<range_value_t<Range>> P>
    constexpr auto erase(Range& range, T const& value, P const& projection) -> typename Range::size_type {
        auto iter = begin(range);
        auto out = iter;
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (project(projection, *iter) != value) {
                *out++ = std::move(*iter);
            }
        }

        auto const erased = last - out;
        range.erase(out, last);
        return erased;
    }

} // namespace up
