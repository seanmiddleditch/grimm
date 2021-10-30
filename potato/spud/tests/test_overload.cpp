// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/overload.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.overload", "[potato][spud]") {
    int result = 0;

    auto o = up::overload(
        [&](int n) { return result += n; },
        [&](float f) { return result = static_cast<int>(static_cast<float>(result) * f); });

    CHECK(o(7) == 7);
    CHECK(o(-3) == 4);
    CHECK(o(2.f) == 8);
    CHECK(o(0.25f) == 2);
    CHECK(o(0) == 2);
}
