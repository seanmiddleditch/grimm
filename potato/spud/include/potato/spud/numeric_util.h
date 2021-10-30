// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {

    template <typename T>
    constexpr T max(T const& lhs, T const& rhs) {
        return lhs < rhs ? rhs : lhs;
    }

    template <typename T>
    constexpr T min(T const& lhs, T const& rhs) {
        return lhs < rhs ? lhs : rhs;
    }

    template <typename T>
    constexpr T clamp(T const& x, T const& min, T const& max) {
        return x < min ? min : x > max ? max : x;
    }

    template <typename IteratorT, typename SentinelT, typename FunctionT, typename ResultT, typename ProjT>
    constexpr ResultT accumulate(
        IteratorT first,
        SentinelT last,
        ResultT initial,
        FunctionT const& op,
        ProjT const& proj) {
        while (first != last) {
            initial = op(initial, proj(*first));
            ++first;
        }
        return initial;
    }

    template <typename IteratorT, typename SentinelT, typename FunctionT, typename ResultT>
    constexpr ResultT accumulate(IteratorT first, SentinelT last, ResultT initial, FunctionT const& op) {
        while (first != last) {
            initial = op(initial, *first);
            ++first;
        }
        return initial;
    }

} // namespace up
