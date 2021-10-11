// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <cinttypes>
#include <cstddef>

namespace up {
    inline namespace types {
        using int8 = std::int8_t;
        using uint8 = std::uint8_t;

        using int16 = std::int16_t;
        using uint16 = std::uint16_t;

        using int32 = std::int32_t;
        using uint32 = std::uint32_t;

        using int64 = std::int64_t;
        using uint64 = std::uint64_t;

        using uint = unsigned int;

        using intptr = std::intptr_t;
        using uintptr = std::uintptr_t;

        using byte = std::byte;

        using size_t = std::size_t;
        using ssize_t = std::ptrdiff_t;
    } // namespace types
} // namespace up
