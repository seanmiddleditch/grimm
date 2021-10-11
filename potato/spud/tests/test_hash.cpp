// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/hash.h"
#include "potato/spud/span.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.hash", "[potato][spud]") {
    using namespace up;

    SECTION("default hash_value") {
        CHECK(hash_value<fnv1a>('x') == 0xaf63f54c86021707);
        CHECK(hash_value<fnv1a>(string_view("hello world")) == 0x779a65e7023cd2e7);
        CHECK(hash_value<fnv1a>(zstring_view("hello world")) == 0x779a65e7023cd2e7);
        CHECK(hash_value<fnv1a>(span<int const>{{1, 2, 3, 4, 5}}) == 0x1916ceffaf539564);
    }

    SECTION("hash_combine") {
        uint64 hash1 = hash_value(7);
        uint64 hash2 = hash_value(-99);

        CHECK(hash1 != hash_combine(hash1, hash2));
        CHECK(hash2 != hash_combine(hash1, hash2));
        CHECK(hash_combine(hash1, hash2) != hash_combine(hash2, hash1));
    }
}
