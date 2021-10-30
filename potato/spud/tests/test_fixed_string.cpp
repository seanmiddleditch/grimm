// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/fixed_string.h"

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("potato.spud.fixed_string", "[potato][spud]") {
    using namespace up;

    SECTION("empty fixed_string") {
        fixed_string<32> fs;

        CHECK(fs.empty());

        CHECK(fs.empty());
        CHECK(fs.capacity() == 31);
        CHECK(*fs.data() == '\0');
    }

    SECTION("fixed_string from string_view") {
        fixed_string<32> fs = string_view("hello world");

        CHECK_FALSE(fs.empty());

        CHECK(fs.size() == 11);
        CHECK(fs.capacity() == 31);
        CHECK(std::strcmp(fs.c_str(), "hello world") == 0);
    }

    SECTION("fixed_string from overlong string_view") {
        fixed_string<5> ss = string_view("hello world");

        CHECK_FALSE(ss.empty());

        CHECK(ss.size() == 4);
        CHECK(ss.capacity() == 4);
        CHECK(std::strcmp(ss.c_str(), "hell") == 0);
    }
}
