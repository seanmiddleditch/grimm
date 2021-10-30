// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "typelist.h"

#include <type_traits>
#include <utility>

namespace up {
    namespace _detail {
        template <typename V, template <typename...> class C, typename... A>
        struct detector : std::false_type {};

        template <template <typename...> class C, typename... A>
        struct detector<std::void_t<C<A...>>, C, A...> : std::true_type {};

        template <typename T>
        struct range_value {
            using type = decltype(*begin(std::declval<T>()));
        };
        template <typename T, int N>
        struct range_value<T[N]> {
            using type = T;
        };
    } // namespace _detail

    template <typename T>
    struct is_contiguous {
        static constexpr bool value = std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;
    };

    template <typename T>
    constexpr bool is_contiguous_v = is_contiguous<T>::value;

    template <typename F>
    struct signature;
    template <typename S>
    struct function_params;
    template <typename S>
    struct function_result;

    template <typename F>
    using signature_t = typename signature<F>::type;

    template <typename R, typename... A>
    struct function_params<R(A...)> {
        using type = up::typelist<A...>;
    };

    template <typename R, typename... A>
    struct function_result<R(A...)> {
        using type = R;
    };

    template <template <class...> class Op, class... Args>
    constexpr bool is_detected_v = _detail::detector<void, Op, Args...>::value;

    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(std::is_same_v<int, remove_cvref_t<int const&>>);

    template <typename T>
    using deref_t = decltype(*std::declval<T>());

    template <typename T>
    using range_value_t = typename _detail::range_value<std::remove_cvref_t<T>>::type;

    template <typename T>
    constexpr bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
    static_assert(is_numeric_v<bool> && is_numeric_v<char> && is_numeric_v<float>);
    static_assert(!is_numeric_v<int*> && !is_numeric_v<float&> && !is_numeric_v<std::nullptr_t>);

    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = std::is_invocable_v<Functor, ParamTypes...>;

    template <typename T, typename Arguments, typename = void>
    struct is_braces_constructible {
        static constexpr bool value = false;
    };

    template <typename T, typename... A>
    struct is_braces_constructible<T, typelist<A...>, std::void_t<decltype(T{std::declval<A>()...})>> {
        static constexpr bool value = true;
    };

    template <typename T, typename... A>
    constexpr bool is_braces_constructible_v = is_braces_constructible<T, typelist<A...>>::value;

    struct any_type {
        template <typename T>
        constexpr operator T();
    };
} // namespace up

#if defined(UP_PLATFORM_WINDOWS)
template <typename R, typename... A>
struct up::signature<R __stdcall(A...)> {
    using type = R(A...);
};
template <typename R, typename... A>
struct up::signature<R __vectorcall(A...)> {
    using type = R(A...);
};
template <typename R, typename... A>
struct up::signature<R(__stdcall*)(A...)> {
    using type = R(A...);
};
template <typename R, typename... A>
struct up::signature<R(__vectorcall*)(A...)> {
    using type = R(A...);
};
template <typename T, typename R, typename... A>
struct up::signature<R (__thiscall T::*)(A...)> {
    using type = R(T&, A...);
};
template <typename T, typename R, typename... A>
struct up::signature<R (__vectorcall T::*)(A...)> {
    using type = R(T&, A...);
};
template <typename T, typename R, typename... A>
struct up::signature<R (__thiscall T::*)(A...) const> {
    using type = R(T const&, A...);
};
template <typename T, typename R, typename... A>
struct up::signature<R (__vectorcall T::*)(A...) const> {
    using type = R(T const&, A...);
};
#else
template <typename R, typename... A>
struct up::signature<R(A...)> {
    using type = R(A...);
};
template <typename R, typename... A>
struct up::signature<R (*)(A...)> {
    using type = R(A...);
};
template <typename T, typename R, typename... A>
struct up::signature<R (T::*)(A...)> {
    using type = R(T&, A...);
};
#endif
