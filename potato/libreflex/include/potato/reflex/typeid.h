// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/nameof.h"

namespace up::reflex {
    class TypeId {
    public:
        template <typename T>
        friend consteval TypeId makeTypeId() noexcept;

        constexpr uint64 raw() const noexcept { return _raw; }

    private:
        explicit constexpr TypeId(uint64 raw) noexcept : _raw(raw) { }

        uint64 _raw = 0;
    };

    template <typename T>
    consteval TypeId makeTypeId() noexcept {
        auto const name = nameof<T>();
        auto const hash = hash_value(name);
        return TypeId{hash};
    }
} // namespace up::reflex
