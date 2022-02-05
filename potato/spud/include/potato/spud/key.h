// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/int_types.h"

#include <compare>

namespace up {
    template <typename Tag, typename Underlying = uint32, Underlying Invalid = Underlying{}>
    class Key {
    public:
        using underlying_type = Underlying;

        constexpr Key() noexcept = default;
        constexpr explicit Key(underlying_type value) noexcept : _value(value) { }

        constexpr static underlying_type invalid = Invalid;

        constexpr explicit operator bool() const noexcept { return _value != Invalid; }

        constexpr bool valid() const noexcept { return _value != Invalid; }
        constexpr underlying_type value() const noexcept { return _value; }

        auto operator<=>(Key const& rhs) const noexcept = default;

        template <typename HashAlgorithm>
        friend constexpr auto hash_append(HashAlgorithm& hasher, Key key) noexcept {
            return hash_append(hasher, key._value);
        };

    private:
        underlying_type _value{Invalid};
    };
} // namespace up
