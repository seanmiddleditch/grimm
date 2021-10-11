// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up {
    class string_view;
    class zstring_view;
    class string;
    template <typename T>
    class vector;
    template <typename T>
    class box;
    template <typename T>
    class rc;
} // namespace up

namespace up::reflex {
    /// True if the provided type is a recognized string type
    template <typename T>
    constexpr bool is_string_v =
        std::is_same_v<T, up::string_view> || std::is_same_v<T, up::zstring_view> || std::is_same_v<T, up::string>;
    static_assert(is_string_v<string_view>);
    static_assert(!is_string_v<char*>);

    /// True if the provided type is an integral or floating point type
    template <typename T>
    constexpr bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
    static_assert(is_numeric_v<bool> && is_numeric_v<char> && is_numeric_v<float>);
    static_assert(!is_numeric_v<int*> && !is_numeric_v<float&> && !is_numeric_v<std::nullptr_t>);

    /// True if the provided type is a vector specialization
    namespace _detail {
        template <typename T>
        struct is_vector {
            constexpr static bool value = false;
        };
        template <typename T>
        struct is_vector<vector<T>> {
            constexpr static bool value = true;
        };
    } // namespace _detail
    template <typename T>
    constexpr bool is_vector_v = _detail::is_vector<T>::value;

    /// True if the provided type is a box specialization
    namespace _detail {
        template <typename T>
        struct is_box {
            constexpr static bool value = false;
        };
        template <typename T>
        struct is_box<box<T>> {
            constexpr static bool value = true;
        };
    } // namespace _detail
    template <typename T>
    constexpr bool is_box_v = _detail::is_box<T>::value;

    /// True if the provided type is an rc specialization
    namespace _detail {
        template <typename T>
        struct is_rc {
            constexpr static bool value = false;
        };
        template <typename T>
        struct is_rc<rc<T>> {
            constexpr static bool value = true;
        };
    } // namespace _detail
    template <typename T>
    constexpr bool is_rc_v = _detail::is_rc<T>::value;
} // namespace up::reflex
