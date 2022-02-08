// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "fixed_string.h"
#include "hash.h"
#include "zstring_view.h"

namespace up {
    template <typename Tag, typename Underlying = decltype(hash_value(""))>
    class StaticId {
    public:
        constexpr StaticId() = default;
        consteval StaticId(const char* string) noexcept : _value(hash_value(string)), _name(string) { }
        template <size_t N>
        consteval StaticId(fixed_string<N> string) noexcept
            : _value(hash_value(string.c_str()))
            , _name(string.c_str()) { }

        auto operator<=>(StaticId const& rhs) const noexcept { return _value <=> rhs._value; }
        auto operator==(StaticId const& rhs) const noexcept { return _value == rhs._value; }
        
        explicit operator bool() const noexcept { return _value != Underlying{}; }

        constexpr bool valid() const noexcept { return _value != Underlying{}; }
        constexpr Underlying value() const noexcept { return _value; }
        constexpr zstring_view name() const noexcept { return _name; }

        template <typename HashAlgorithm>
        friend constexpr auto hash_append(HashAlgorithm& hasher, StaticId id) noexcept {
            return hash_append(hasher, id._value);
        };

    private:
        Underlying _value = {};
        const char* _name = "";
    };
} // namespace up
