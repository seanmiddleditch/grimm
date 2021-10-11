// https://stackoverflow.com/a/20170989
// By Howard Hinnant

#pragma once

#include "potato/spud/string_view.h"

#include <string_view>

namespace up {
    template <class T>
    constexpr string_view nameof() {
#ifdef __clang__
        constexpr string_view p = __PRETTY_FUNCTION__;
        return p.substr(34, p.size() - 34 - 1);
#elif defined(__GNUC__)
        constexpr string_view p = __PRETTY_FUNCTION__;
        return p.substr(49, p.size() - 49 - 1);
#elif defined(_MSC_VER)
        constexpr string_view p = __FUNCSIG__;
        return p.substr(41, p.size() - 41 - 7);
#endif
    }
} // namespace up
