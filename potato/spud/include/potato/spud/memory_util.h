// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"

#include <cstring>
#include <utility>

namespace up {
    template <typename InputIt, typename SizeT>
    void default_construct_n(InputIt first, SizeT count);

    template <typename InputIt, typename SizeT, typename TypeT>
    void uninitialized_value_construct_n(InputIt first, SizeT count, TypeT const& value);

    template <typename InputIt, typename SizeT>
    void destruct_n(InputIt first, SizeT count);

    template <typename InputIt, typename SizeT, typename TypeT>
    void unitialized_copy_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void copy_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void unitialized_move_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void move_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void move_backwards_n(InputIt first, SizeT count, TypeT* out_last);
} // namespace up

template <typename InputIt, typename SizeT>
void up::default_construct_n(InputIt first, SizeT count) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (!std::is_trivially_constructible_v<type>) {
        while (count-- > 0) {
            new (first++) type{};
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::uninitialized_value_construct_n(InputIt first, SizeT count, TypeT const& value) {
    using type = std::remove_reference_t<decltype(*first)>;
    while (count-- > 0) {
        new (first++) type(value);
    }
}

template <typename InputIt, typename SizeT>
void up::destruct_n(InputIt first, SizeT count) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (!std::is_trivially_destructible_v<type>) {
        for (SizeT i = 0; i != count; ++i, ++first) {
            first->~type();
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::unitialized_copy_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_constructible_v<TypeT, type> && std::is_pointer_v<InputIt>) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        while (count-- > 0) {
            new (out_first++) TypeT(*first++);
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::copy_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_assignable_v<TypeT, type> && std::is_pointer_v<InputIt>) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        while (count-- > 0) {
            *out_first++ = *first++;
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::unitialized_move_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    using type_rvalue = type&&;
    if constexpr (std::is_trivially_constructible_v<TypeT, type_rvalue> && std::is_pointer_v<InputIt>) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        while (count-- > 0) {
            new (out_first++) TypeT(std::move(*first++));
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::move_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    using type_rvalue = type&&;
    if constexpr (std::is_trivially_assignable_v<TypeT, type_rvalue> && std::is_pointer_v<InputIt>) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        while (count-- > 0) {
            *out_first++ = std::move(*first++);
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::move_backwards_n(InputIt first, SizeT count, TypeT* out_last) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_assignable_v<TypeT, type&&> && std::is_pointer_v<InputIt>) {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        std::memmove(out_last - count, first - count, count * sizeof(type));
    }
    else {
        for (auto in = first + count; in != first;) {
            *out_last-- = std::move(*--in);
        }
    }
}
