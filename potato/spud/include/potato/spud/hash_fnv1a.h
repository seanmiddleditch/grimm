// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "int_types.h"
#include "platform.h"

#if defined(UP_COMPILER_MICROSOFT)
// https://stackoverflow.com/questions/37658794/integer-constant-overflow-warning-in-constexpr
__pragma(warning(disable : 4307))
#endif

    namespace up {
    /// <summary> A uhash-compatible fnv1-a hasher. </summary>
    class fnv1a {
    public:
        using result_type = uint64;

        constexpr void append_bytes(char const* data, size_t size) noexcept;
        constexpr result_type finalize() const noexcept { return _state; }

    private:
        static constexpr uint64 offset = 14695981039346656037ULL;

        result_type _state = offset;
    };

    UP_FORCEINLINE constexpr void fnv1a::append_bytes(char const* data, size_t size) noexcept {
        constexpr uint64 prime = 1099511628211ULL;

        for (size_t i = 0; i != size; ++i) {
            _state ^= static_cast<uint64>(data[i]);
            _state *= prime;
        }
    }
} // namespace up
