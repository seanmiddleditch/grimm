// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/fixed_string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/hash.h"

#include <catch2/catch.hpp>
#include <iostream>

namespace {
    template <typename Fixed>
    consteval size_t length(Fixed const& fixed) {
        return fixed.size();
    }
} // namespace

TEST_CASE("potato.spud.fixed_string", "[potato][spud]") {
    using namespace up;

    SECTION("empty fixed_string") {
        fixed_string const empty = "";

        CHECK(empty.empty());
        CHECK(*empty.data() == '\0');
    }

    SECTION("init fixed_string") {
        constexpr fixed_string test = "This is a test";
        CHECK(!test.empty());
        CHECK(length(test) == 14);
        CHECK(string_view{"This is a test"} == test.c_str());
    }

    SECTION("compare fixed_string") {
        constexpr fixed_string test1 = "This is a test";
        constexpr fixed_string test2 = "This is a test";
        constexpr fixed_string test3 = "This is different";
        CHECK(test1 == test2);
        CHECK(test1 != test3);
    }

    SECTION("hash fixed_string") {
        auto const fhash = hash_value(fixed_string{"hash this"});
        auto const shash = hash_value("hash this");
        auto const vhash = hash_value(string_view{"hash this"});
        auto const whash = hash_value(fixed_string{"hash this different thing"});

        CHECK(fhash == shash);
        CHECK(fhash == vhash);
        CHECK(whash != fhash);
    }
}
