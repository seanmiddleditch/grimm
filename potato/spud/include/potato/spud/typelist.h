// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up {
    template <typename... T>
    struct typelist {};

    namespace _detail {
        template <typename U>
        struct typelist_head;
        template <typename T, typename... R>
        struct typelist_head<typelist<T, R...>> {
            using type = T;
        };

        template <typename U>
        struct typelist_tail;
        template <typename T, typename... R>
        struct typelist_tail<typelist<T, R...>> {
            using type = typelist<R...>;
        };

        template <template <class...> class T, typename U>
        struct typelist_apply;
        template <template <class...> class T, typename... U>
        struct typelist_apply<T, typelist<U...>> {
            using type = T<U...>;
        };

        template <template <class...> class T, typename U>
        struct typelist_map;
        template <template <class...> class T, typename... U>
        struct typelist_map<T, typelist<U...>> {
            using type = typelist<T<U>...>;
        };

        template <int N, typename... U>
        struct type_at_impl;
        template <typename T, typename... U>
        struct type_at_impl<0, typelist<T, U...>> {
            using type = T;
        };
        template <int N, typename T, typename... U>
        struct type_at_impl<N, typelist<T, U...>> {
#if defined(__has_builtin)
#    if __has_builtin(__type_pack_element)
#        define UP_HAS_TYPE_PACK_ELEMENT
#    endif
#endif
#if defined(UP_HAS_TYPE_PACK_ELEMENT)
            using type = __type_pack_element<N, T, U...>;
#else
            using type = typename type_at_impl<N - 1, typelist<U...>>::type;
#endif
#undef UP_HAS_TYPE_PACK_ELEMENT
        };
    } // namespace _detail

    template <typename T>
    using typelist_head_t = typename _detail::typelist_head<T>::type;

    static_assert(std::is_same_v<typelist_head_t<typelist<int, float, char>>, int>);

    template <typename T>
    using typelist_tail_t = typename _detail::typelist_tail<T>::type;

    static_assert(std::is_same_v<typelist_tail_t<typelist<int, float, char>>, typelist<float, char>>);

    // Causing problems with Sphinx
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
    template <typename T>
    constexpr std::size_t typelist_size_v = 0;
    template <typename... T>
    constexpr std::size_t typelist_size_v<typelist<T...>> = sizeof...(T);

    static_assert(typelist_size_v<typelist<>> == 0);
    static_assert(typelist_size_v<typelist<int>> == 1);
    static_assert(typelist_size_v<typelist<int, float, char>> == 3);
#endif

    template <template <class...> class T, typename U>
    using typelist_apply_t = typename _detail::typelist_apply<T, U>::type;

    static_assert(typelist_apply_t<std::is_same, typelist<int, int>>::value);

    template <template <class...> class T, typename U>
    using typelist_map_t = typename _detail::typelist_map<T, U>::type;

    static_assert(
        std::is_same_v<typelist_map_t<std::add_const_t, typelist<int, char>>, typelist<int const, char const>>);

    template <int N, typename T>
    using typelist_at_t = typename _detail::type_at_impl<N, T>::type;

    static_assert(std::is_same_v<int, typelist_at_t<0, typelist<int, char, bool, float, int>>>);
    static_assert(std::is_same_v<bool, typelist_at_t<2, typelist<int, char, bool, float, int>>>);

} // namespace up
