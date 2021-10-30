// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "traits.h"
#include "utility.h"

#include <cstdint>

namespace up {
    template <range Range, equality_comparable_with<range_value_t<Range>> T>
    constexpr auto find(Range const& range, T const& value) noexcept(noexcept(*begin(range) == value)) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (*iter == value) {
                return iter;
            }
        }

        return last;
    }

    template <range Range, typename T, typename E = equality, projection<range_value_t<Range>> P = identity>
    constexpr auto find(Range const& range, T const& value, E const& equals, P const& projection) noexcept(
        noexcept(equals(project(projection, *begin(range)), value))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (equals(project(projection, *iter), value)) {
                return iter;
            }
        }

        return last;
    }

    template <range Range, typename T, typename E = equality>
    constexpr auto find(Range const& range, T const& value, E const& equals) noexcept(
        noexcept(equals(*begin(range), value))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (equals(*iter, value)) {
                return iter;
            }
        }

        return last;
    }

    template <range Range, predicate<range_value_t<Range>> P = identity>
    constexpr auto find_if(Range const& range, P const& predicate) noexcept(noexcept(predicate(*begin(range)))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (predicate(*iter)) {
                return iter;
            }
        }

        return last;
    }

    template <range Range, equality_comparable_with<range_value_t<Range>> T>
    constexpr auto contains(Range const& range, T const& value) noexcept(noexcept(*begin(range) == value)) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (*iter == value) {
                return true;
            }
        }

        return false;
    }

    template <range Range, typename T, typename E = equality, projection<range_value_t<Range>> P = identity>
    constexpr auto contains(Range const& range, T const& value, E const& equals, P const& projection) noexcept(
        noexcept(equals(project(projection, *begin(range)), value))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (equals(project(projection, *iter), value)) {
                return true;
            }
        }

        return false;
    }

    template <range Range, typename T, typename E = equality>
    constexpr auto contains(Range const& range, T const& value, E const& equals) noexcept(
        noexcept(equals(*begin(range), value))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (equals(*iter, value)) {
                return true;
            }
        }

        return false;
    }

    template <range Range, predicate<range_value_t<Range>> P>
    constexpr auto any(Range const& range, P const& predicate) noexcept(noexcept(predicate(*begin(range)))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (predicate(*iter)) {
                return true;
            }
        }

        return false;
    }

    template <range Range, predicate<range_value_t<Range>> P>
    constexpr auto all(Range const& range, P const& predicate) noexcept(noexcept(predicate(*begin(range)))) {
        auto iter = begin(range);
        auto last = end(range);

        for (; iter != last; ++iter) {
            if (!predicate(*iter)) {
                return false;
            }
        }

        return true;
    }

} // namespace up
