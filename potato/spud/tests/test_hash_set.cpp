// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/hash_set.h"
#include "potato/spud/string.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.hash_set", "[potato][spud]") {
    using namespace up;

    SECTION("fill") {
        hash_set<string> values;

        CHECK(values.insert("test"));
        CHECK_FALSE(values.insert("test"));
        CHECK(values.insert("bob"));

        REQUIRE(values.contains("test"));
        REQUIRE(values.contains("bob"));
        CHECK_FALSE(values.contains(""));

        CHECK(2 == values.size());

        // we expect only a single group
        CHECK(16 == values.capacity());
    }

    SECTION("fill large") {
        hash_set<int> values;

        constexpr int count = 16;

        for (int i = 0; i != count; ++i) {
            CHECK(values.insert(i));
        }

        CHECK(values.size() == count);

        for (int i = 0; i != count; ++i) {
            CHECK(values.contains(i));
        }
    }

    SECTION("erase") {
        hash_set<int> values;

        constexpr int count = 2'048;
        static_assert((count & (count - 1)) == 0);

        for (int i = 0; i != count; ++i) {
            values.insert(i);
        }

        for (int i = 0; i < count; i += 2) {
            REQUIRE(values.erase(i));
        }

        CHECK(values.size() == count / 2);

        for (int i = 1; i < count; i += 2) {
            REQUIRE(values.contains(i));
        }

        for (int i = 1; i < count; i += 2) {
            values.erase(i);
        }

        CHECK(values.empty());
    }
}
