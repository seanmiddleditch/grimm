// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "utility.h"

#include <algorithm>

namespace up {
    template <typename Range, typename Compare = less, typename Projection = identity>
    constexpr void sort(Range&& range, Compare const& compare = {}, Projection const& projection = {}) noexcept(
        noexcept(project(projection, *begin(range)))) {
        auto first = begin(range);
        auto last = end(range);

        auto comparer = [&](auto const& lhs, auto const& rhs) {
            return compare(project(projection, lhs), project(projection, rhs));
        };

        std::sort(first, last, comparer);
    }
} // namespace up
