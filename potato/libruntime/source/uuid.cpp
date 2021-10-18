// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/uuid.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/ascii.h"
#include "potato/spud/string_writer.h"

#include <nanofmt/format.h>

up::UUID::UUID(up::byte const* bytes, size_t length) noexcept : _data{HighLow{}} {
    UP_ASSERT(length == octects);
    for (unsigned i = 0; i != octects; ++i) {
        _data.ub[i] = bytes[i];
    }
}

constexpr char byteToString(up::byte byte) noexcept {
    return static_cast<char>(byte);
}

auto up::UUID::toString() const -> string {
    char chars[strLength] = {};
    nanofmt::format_to(chars, "{}", *this);
    return chars;
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
            if (bidx == sizeof(result._data.ub)) {
                return UUID{};
            }
            result._data.ub[bidx++] = next;
        }

        octect = !octect;
    }

    if (bidx != sizeof(result._data.ub) || octect) {
        return UUID{};
    }

    return result;
}
