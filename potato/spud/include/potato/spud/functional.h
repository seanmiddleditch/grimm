// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"

#include <functional>
#include <utility>

namespace up {
#if defined(__cpp_lib_constexpr_invoke)
    using std::invoke;
#else
    template <class Class, class Return, class First, class... Rest>
    constexpr auto invoke(Return Class::*func, First&& first, Rest&&... rest)
        -> std::invoke_result_t<decltype(func), First, Rest...> {
        if constexpr (std::is_member_function_pointer_v<decltype(func)>) {
            return (std::forward<First>(first).*func)(std::forward<Rest>(rest)...);
        }
        else {
            return func(std::forward<First>(first), std::forward<Rest>(rest)...);
        }
    }

    template <typename Functor, typename... Args>
    constexpr auto invoke(Functor&& func, Args&&... args)
        -> decltype(std::forward<Functor>(func)(std::forward<Args>(args)...)) {
        return std::forward<Functor>(func)(std::forward<Args>(args)...);
    }
#endif
} // namespace up
