// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up {
    template <typename T, typename U>
    concept same_as = std::is_same_v<T, U>;

    template <typename T, typename U>
    concept convertible_to = requires(T const& t) {
        static_cast<U>(t);
    };

    template <typename T, typename U>
    concept derived_from = std::is_base_of_v<U, T>;

    template <typename T, typename U>
    concept equality_comparable_with = requires(T t, U u) {
        { t == u }
        ->convertible_to<bool>;
    };

    template <typename T, typename U>
    concept less_than_comparable_with = requires(T t, U u) {
        { t < u }
        ->convertible_to<bool>;
    };

    template <typename F, typename... Args>
    concept callable = std::is_invocable_v<std::remove_reference_t<F>, Args...>;

    template <typename F, typename R, typename... Args>
    concept callable_r = (callable<F, Args...> && std::is_convertible_v<std::invoke_result_t<F, Args...>, R>);

    template <typename F, typename... Args>
    concept predicate = callable_r<F, bool, Args...>;

    template <typename E>
    concept enumeration = std::is_enum_v<E>;

    template <typename T>
    concept integral = std::is_integral_v<T>;

    template <typename R>
    concept range = std::is_array_v<std::remove_cv_t<std::remove_reference_t<R>>> || requires(R rng) {
        typename R::value_type;
        typename R::size_type;
        rng.begin();
        rng.end();
    };

    template <typename P, typename T>
    concept projection = callable<P, T> || requires(T const& v, P p) {
        v.*p;
    }
    || requires(T const* v, P p) { v->*p; };
} // namespace up
