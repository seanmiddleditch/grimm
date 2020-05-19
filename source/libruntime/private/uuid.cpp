// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/ascii.h"

constexpr up::UUID::UUID(Bytes const& bytes) noexcept : _data{HighLow{}} {
    for (int i = 0; i != 16; ++i) {
        _data.ub[i] = bytes[i];
    }
}

constexpr char byteToString(up::byte byte) noexcept {
    return static_cast<char>(byte);
}

auto up::UUID::toString() const -> string {
    // format 9554084e-4100-4098-b470-2125f5eed133
    string_writer buffer;
    format_append(buffer, "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                  _data.ub[0], _data.ub[1], _data.ub[2], _data.ub[3],
                  _data.ub[4], _data.ub[5],
                  _data.ub[6], _data.ub[7],
                  _data.ub[8], _data.ub[9],
                  _data.ub[10], _data.ub[11], _data.ub[12], _data.ub[13], _data.ub[14], _data.ub[15]);

    return buffer.c_str();
}

auto up::UUID::fromString(string_view id) noexcept -> UUID {
    if (!id.empty() && id.front() == '{' && id.back() == '}') {
        id = id.substr(1, id.size() - 2);
    }

    byte next = {};
    bool octect = false;

    UUID result;
    int bidx = 0;

    for (auto c : id) {
        if (c == '-') {
            continue;
        }

        int digit = ascii::from_hex(c);
        if (digit == -1) {
            return UUID{};
        }

        next <<= 4;
        next |= static_cast<byte>(digit);

        if (octect) {
            if (bidx == 16) {
                return UUID{};
            }
            result._data.ub[bidx++] = next;
        }

        octect = !octect;
    }

    if (bidx != 16 || octect) {
        return UUID{};
    }

    return result;
}
