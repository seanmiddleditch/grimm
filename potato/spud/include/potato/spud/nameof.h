// https://stackoverflow.com/a/20170989
// By Howard Hinnant

#pragma once

#include "potato/spud/fixed_string.h"
#include "potato/spud/string_view.h"

#include <compare>

namespace up {
    namespace _detail::nameof {
#if defined(__clang__) || defined(__GNUC__)
        consteval char const* find_equal_reverse(char const* str) {
            while (*str != '=')
                --str;
            return str;
        }
#elif defined(_MSC_VER)
        // given a string like "whatever<> outer<inner<innermost>>", finds
        // the substring after the first of the nested pairs < > starting
        // from the end; e.g. "inner<innermost>>" in the example
        consteval char const* find_tpl_start(char const* str) {
            int nest = 0;
            for (;;) {
                if (*str == '>') {
                    ++nest;
                }
                else if (*str == '<' && --nest <= 0) {
                    return str + 1;
                }
                --str;
            }
        }
#endif
    } // namespace _detail::nameof

    template <class T>
    consteval auto nameof() {
#if defined(__clang__) || defined(__GNUC__)
        // form is: blah [with T = $$$]
        // so we trim off the last two characters (NUL byte and ]) and then
        // search backwards for the =, and we know the type starts two
        // characters after that
        constexpr char const* end = __PRETTY_FUNCTION__ + sizeof __PRETTY_FUNCTION__ - 2 /*], NUL*/;
        constexpr char const* func = _detail::nameof::find_equal_reverse(end) + 2;
        return fixed_string<end - func>(func);
#elif defined(_MSC_VER)
        // form is: blah <$$$>(void)
        // so we trim off the last 8 characters (NUL byte, "(void)", and >)
        // and then search backwards to find the beginning of any nested
        // template < > pairs
        constexpr char const* end = __FUNCSIG__ + sizeof __FUNCSIG__ - 8 /*>(void), NUL*/;
        constexpr char const* func = _detail::nameof::find_tpl_start(end);
        return fixed_string<end - func>(func);
#endif
    }
} // namespace up
