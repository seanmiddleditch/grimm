// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up::ascii {
    constexpr auto is_digit(char const ch) noexcept -> bool { return ch >= '0' && ch <= '9'; }

    constexpr auto is_alpha(char const ch) noexcept -> bool {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    constexpr auto is_alnum(char const ch) noexcept -> bool { return is_digit(ch) || is_alpha(ch); }

    constexpr auto is_hex_digit(char const ch) noexcept -> bool {
        return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }

    constexpr auto from_hex(char const ch) noexcept -> int {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return -1;
    }

    constexpr auto toUppercase(char const ch) noexcept -> char {
        return ch >= 'a' && ch <= 'z' ? static_cast<char>(ch - 'a' + 'A') : ch;
    }

    constexpr auto toLowercase(char const ch) noexcept -> char {
        return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch - 'A' + 'a') : ch;
    }
} // namespace up::ascii
