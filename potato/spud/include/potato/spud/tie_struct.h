// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"
#include "typelist.h"

#include <tuple>

namespace up {
    // Binds a structure into a std::tuple to access its individual members
    //
    // Note: this only support structures up to a certain number of members. Extending
    // this to support "larger" structures is easy, if needed.
    //
    template <typename Struct>
    constexpr auto tie_struct(Struct&& s) noexcept {
        if constexpr (
            is_braces_constructible_v<Struct, any_type, any_type, any_type, any_type, any_type, any_type, any_type>) {
            auto&& [a, b, c, d, e, f, g] = s;
            return std::tie(a, b, c, d, e, f, g);
        }
        else if constexpr (
            is_braces_constructible_v<Struct, any_type, any_type, any_type, any_type, any_type, any_type>) {
            auto&& [a, b, c, d, e, f] = s;
            return std::tie(a, b, c, d, e, f);
        }
        else if constexpr (is_braces_constructible_v<Struct, any_type, any_type, any_type, any_type, any_type>) {
            auto&& [a, b, c, d, e] = s;
            return std::tie(a, b, c, d, e);
        }
        else if constexpr (is_braces_constructible_v<Struct, any_type, any_type, any_type, any_type>) {
            auto&& [a, b, c, d] = s;
            return std::tie(a, b, c, d);
        }
        else if constexpr (is_braces_constructible_v<Struct, any_type, any_type, any_type>) {
            auto&& [a, b, c] = s;
            return std::tie(a, b, c);
        }
        else if constexpr (is_braces_constructible_v<Struct, any_type, any_type>) {
            auto&& [a, b] = s;
            return std::tie(a, b);
        }
        else if constexpr (is_braces_constructible_v<Struct, any_type>) {
            auto&& [a] = s;
            return std::tie(a);
        }
        else {
            return std::tuple<>();
        }
    }

    // Helper to extract the types of a tuple as a typelist
    //
    template <typename T>
    struct tuple_to_typelist;

    template <typename... T>
    struct tuple_to_typelist<std::tuple<T...>> {
        using type = typelist<T...>;
    };

    template <typename T>
    using tuple_to_typelist_t = typename tuple_to_typelist<T>::type;

    // Retrieves the types of members of a structure as a typelist
    // FIXME: this will never return references even when struct members are references!
    //
    template <typename Struct>
    using member_typelist_t = typelist_map_t<
        std::remove_reference_t,
        tuple_to_typelist_t<decltype(tie_struct<Struct>(std::declval<Struct>()))>>;
} // namespace up
