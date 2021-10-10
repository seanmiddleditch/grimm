// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/hash.h"
#include "potato/spud/string.h"

#include <nanofmt/format.h>
#include <array>

namespace up {
    class UP_RUNTIME_API UUID {
    public:
        static constexpr int octects = 16;
        static constexpr int strLength = 37; // 32 hex values, four dashes, and NUL
        using Bytes = up::byte[octects];

        constexpr UUID() noexcept : _data{HighLow{}} {}
        constexpr UUID(UUID const& rhs) noexcept : _data{rhs._data.u64} {}
        explicit UUID(Bytes const& bytes) noexcept : UUID(bytes, sizeof(bytes)) {}
        UUID(up::byte const* bytes, size_t length) noexcept;

        constexpr bool isValid() const noexcept { return _data.u64.high != 0 || _data.u64.low != 0; }

        constexpr bool operator==(UUID const& rhs) const noexcept {
            return _data.u64.high == rhs._data.u64.high && _data.u64.low == rhs._data.u64.low;
        }
        constexpr bool operator!=(UUID const& rhs) const noexcept {
            return _data.u64.high != rhs._data.u64.high || _data.u64.low != rhs._data.u64.low;
        }

        auto toString() const -> string;

        up::byte const* bytes() const noexcept { return _data.ub; }

        template <typename HashAlgorithm = default_hash>
        friend uint64 hash_value(UUID uuid) noexcept {
            HashAlgorithm hasher{};
            hash_append(hasher, uuid._data.u64.high);
            hash_append(hasher, uuid._data.u64.low);
            return hasher.finalize();
        }

        static auto generate() noexcept -> UUID;
        static auto fromString(string_view id) noexcept -> UUID;

    private:
        struct HighLow {
            uint64 high = 0;
            uint64 low = 0;
        };

        union Storage {
            HighLow u64 = {};
            byte ub[octects];
        };

        Storage _data = {HighLow{}};
    };

    static_assert(sizeof(UUID) == UUID::octects, "sizeof(uuid) must be 16 octects");

} // namespace up

namespace nanofmt {
    template <>
    struct formatter<up::UUID> : nanofmt::formatter<char const*> {
        void format(up::UUID const& uuid, format_output& out) {
            // format 9554084e-4100-4098-b470-2125f5eed133
            up::byte const* const bytes = uuid.bytes();
            format_to(
                out,
                "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                bytes[0],
                bytes[1],
                bytes[2],
                bytes[3],
                bytes[4],
                bytes[5],
                bytes[6],
                bytes[7],
                bytes[8],
                bytes[9],
                bytes[10],
                bytes[11],
                bytes[12],
                bytes[13],
                bytes[14],
                bytes[15]);
        }
    };
} // namespace nanofmt
