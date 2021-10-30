// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/find.h"
#include "potato/spud/vector.h"

#include <catch2/catch.hpp>
#include <iostream>

namespace {
    constexpr bool is_even(int val) noexcept { return val % 2 == 0; }
    constexpr bool is_negative(int val) noexcept { return val < 0; }
} // namespace

TEST_CASE("potato.spud.find", "[potato][spud]") {
    using namespace up;

    int array[] = {4, 7, 9, 2, 1, 0, 900};
    vector vec{-88, 4, 0, 7};

    SECTION("find") {
        CHECK(find(array, 4) == std::begin(array));
        CHECK(find(vec, -88) == std::begin(vec));

        CHECK(find(array, 900) == std::end(array) - 1);
        CHECK(find(vec, 7) == std::end(vec) - 1);

        CHECK(find(array, 12948734) == std::end(array));
        CHECK(find(vec, 12948734) == std::end(vec));
    }

    SECTION("find_if") {
        CHECK(find_if(array, is_even) == std::begin(array));
        CHECK(find_if(vec, is_negative) == std::begin(vec));

        CHECK(find_if(array, [](int i) { return i > 100; }) == std::end(array) - 1);
        CHECK(find_if(vec, [](int i) { return !is_even(i); }) == std::end(vec) - 1);

        CHECK(find_if(array, [](int) { return false; }) == std::end(array));
        CHECK(find_if(vec, [](int) { return false; }) == std::end(vec));
    }

    SECTION("any") {
        CHECK_FALSE(any(array, is_negative));
        CHECK(any(vec, is_negative));
    }

    SECTION("all") {
        CHECK(all(array, [](int i) { return !is_negative(i); }));
        CHECK_FALSE(all(vec, is_negative));
    }

    SECTION("contains") {
        CHECK(contains(array, 2));
        CHECK_FALSE(contains(array, 100));
    }
}
