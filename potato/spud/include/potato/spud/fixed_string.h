// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "int_types.h"

#include <compare>

namespace up {
    template <size_t Size>
    class fixed_string;

    template <size_t Size>
    fixed_string(char const (&)[Size]) -> fixed_string<Size - 1 /*NUL*/>;

    template <size_t Size>
    class fixed_string {
    public:
        using value_type = char;
        using pointer = char const*;
        using size_type = size_t;

        // this is "safe" because it's consteval and will fail
        // compilation instead of triggering UB if it reads
        // out-of-bounds
        consteval fixed_string(char const* string) noexcept;

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return Size != 0; }
        [[nodiscard]] constexpr bool empty() const noexcept { return Size == 0; }

        [[nodiscard]] constexpr size_type size() const noexcept { return Size; }

        [[nodiscard]] constexpr pointer data() const noexcept { return _data; }
        [[nodiscard]] constexpr pointer c_str() const noexcept { return _data; }

        [[nodiscard]] constexpr value_type operator[](size_type index) const noexcept { return _data[index]; }

        [[nodiscard]] constexpr friend auto operator<=>(fixed_string const&, fixed_string const&) noexcept = default;

        [[nodiscard]] constexpr friend auto operator==(fixed_string const& lhs, fixed_string const& rhs) noexcept {
            for (size_t i = 0; i != Size; ++i) {
                if (lhs._data[i] != rhs._data[i]) {
                    return false;
                }
            }
            return true;
        }

        template <size_t OtherSize>
        [[nodiscard]] constexpr friend bool operator==(fixed_string const&, fixed_string<OtherSize> const&) noexcept {
            return false;
        }

        template <typename Writer, typename Spec>
        friend void format_value(Writer& writer, fixed_string const& fs, Spec const& options) noexcept {
            format_value_to(writer, {fs._data, Size}, options);
        }

    private:
        char _data[Size + 1 /*NUL*/] = {};
    };

    template <size_t Size>
    consteval fixed_string<Size>::fixed_string(char const* string) noexcept {
        for (size_t i = 0; i != Size; ++i) {
            _data[i] = string[i];
        }
        _data[Size] = '\0';
    }

    template <typename HashAlgorithm, size_t Size>
    constexpr void hash_append(HashAlgorithm& hasher, fixed_string<Size> const& string) noexcept {
        hasher.append_bytes(string.data(), string.size());
    }
} // namespace up
