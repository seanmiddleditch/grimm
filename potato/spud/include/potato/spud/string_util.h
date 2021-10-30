// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "ascii.h"
#include "int_types.h"
#include "platform.h"

namespace up {
    constexpr size_t stringLength(char const* str) noexcept { return __builtin_strlen(str); }

    template <size_t N>
    constexpr size_t stringLength(char const (&str)[N]) noexcept {
        for (size_t i = 0; i != N; ++i) {
            if (str[i] == '\0') {
                return i;
            }
        }
        return N;
    }

    constexpr int stringCompare(char const* left, char const* right, size_t length) noexcept {
        return __builtin_memcmp(left, right, length);
    }

    constexpr char const* stringFindChar(char const* str, size_t length, char ch) noexcept {
#if defined(UP_COMPILER_GCC)
        return (char const*)__builtin_memchr(str, ch, length);
#else
        return __builtin_char_memchr(str, ch, length);
#endif
    }

    inline auto as_char(char8_t const* u8str) -> char const* { return reinterpret_cast<char const*>(u8str); }

    constexpr auto stringEqualNoCase(char const* lstr, char const* rstr, size_t len) noexcept -> bool {
        for (char const* const end = lstr + len; lstr != end; ++lstr, ++rstr) {
            if (ascii::toLowercase(*lstr) != ascii::toLowercase(*rstr)) {
                return false;
            }
        }
        return true;
    }

    constexpr auto stringIndexOfNoCase(
        char const* const haystack,
        size_t const haystackLen,
        char const* const needle,
        size_t const needleLen) noexcept -> ssize_t {
        if (needleLen > haystackLen) {
            return -1;
        }
        if (needleLen == 0) {
            return 0;
        }

        for (ssize_t index = 0, end = static_cast<ssize_t>(haystackLen - needleLen); index <= end; ++index) {
            if (stringEqualNoCase(haystack + index, needle, needleLen)) {
                return index;
            }
        }

        return -1;
    }

} // namespace up
