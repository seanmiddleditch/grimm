// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    namespace _detail {
        template <template <auto, auto> typename F, auto... V>
        struct reduce_t;

        template <template <auto, auto> typename F, auto V>
        struct reduce_t<F, V> {
            static constexpr auto value = V;
        };

        template <template <auto, auto> typename F, auto V, auto... R>
        struct reduce_t<F, V, R...> {
            static constexpr auto value = F<V, reduce_t<F, R...>::value>::value;
        };
    } // namespace _detail

    // Stores an integer sequence of offsets
    //
    template <typename T, T... Values>
    struct value_list {};

    // Fetches a value at a specific index from a value_list
    //
    template <int N, typename Values>
    struct value_at;
    template <typename T, T Value, T... Rest>
    struct value_at<0, value_list<T, Value, Rest...>> {
        static constexpr T value = Value;
    };
    template <typename T, int N, T Value, T... Rest>
    struct value_at<N, value_list<T, Value, Rest...>> {
        static constexpr T value = value_at<N - 1, value_list<T, Rest...>>::value;
    };

    template <int N, typename Values>
    constexpr auto value_at_v = value_at<N, Values>::value;

    template <auto L, auto R>
    struct max_f {
        static constexpr auto value = L < R ? R : L;
    };

    template <auto L, auto R>
    struct min_f {
        static constexpr auto value = L < R ? L : R;
    };

    template <auto L, auto R>
    struct sum_f {
        static constexpr auto value = L + R;
    };

    template <template <auto, auto> typename F, auto... V>
    using reduce_t = typename _detail::reduce_t<F, V...>;

    template <template <auto, auto> typename F, auto... V>
    constexpr auto reduce_v = reduce_t<F, V...>::value;

    static_assert(reduce_v<max_f, -2, 3, 1> == 3);

    template <auto... V>
    constexpr auto max_v = reduce_v<max_f, V...>;

    static_assert(max_v<1, -7, 3> == 3);
    static_assert(max_v<-3, 7, 1> == 7);

    template <auto... V>
    constexpr auto min_v = reduce_v<min_f, V...>;

    static_assert(min_v<1, -7, 3> == -7);
    static_assert(min_v<1, 2, 3> == 1);

    template <auto... V>
    constexpr auto sum_v = reduce_v<sum_f, V...>;

    static_assert(sum_v<1, -7, 3> == -3);
    static_assert(sum_v<1, -7, -3> == -9);
} // namespace up
