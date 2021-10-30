// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/uuid.h"

#include <catch2/catch.hpp>
#include <array>

TEST_CASE("potato.runtime.UUID", "[potato][runtime]") {
    using namespace up;

    SECTION("basic uuid") {
        UUID zero = UUID();
        CHECK(zero == UUID{});
        CHECK(zero.toString() == "00000000-0000-0000-0000-000000000000");

        UUID a = UUID::generate();
        CHECK(a.isValid());

        string str = a.toString();
        UUID b = UUID::fromString(str);
        CHECK(a == b);

        const UUID c = UUID::fromString("00000000-0000-0000-0000-000000000000");
        CHECK(c == UUID{});

        // check invalid length of string
        const UUID d = UUID::fromString("9554084e-4100-4098-b470-2125f5eed13");
        CHECK(d == UUID{});
        const UUID e = UUID::fromString("9554084e-4100-4098-b470-2125f5eed133f");
        CHECK(e == UUID{});
        const UUID f = UUID::fromString("955408-4e4100-4098-b470-2125f5eed133f");
        CHECK(f == UUID{});
        const UUID g = UUID::fromString("9554084e-41K0-4098-PODR-25f5eed133fN");
        CHECK(g == UUID{});

        const UUID h = UUID::fromString("{10000000-0000-0000-0000-000000000002}");
        CHECK(h.isValid());

        string_view const input = "9554084e-4100-4098-b470-2125f5eed133";
        const UUID i = UUID::fromString(input);
        string const output = i.toString();
        CHECK(static_cast<string_view>(output) == input);
    }
}
