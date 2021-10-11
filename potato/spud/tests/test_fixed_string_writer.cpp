// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/fixed_string_writer.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.fixed_string_writer", "[potato][spud]") {
    using namespace up;

    SECTION("default initialization") {
        fixed_string_writer<32> sw;

        static_assert(sizeof(sw) <= 40 /* buffer + 8-byte size */);

        CHECK(sw.empty());
        CHECK(sw.empty());
        CHECK(sw.capacity() == 31);
        CHECK(sw.c_str() == ""_sv);
    }

    SECTION("write") {
        fixed_string_writer<32> sw;

        sw.append("hello");
        sw.append(',');
        sw.append(' ');
        sw.append("world");

        CHECK_FALSE(sw.empty());
        CHECK(sw.size() == 12);
        CHECK(sw.c_str() == "hello, world"_sv);
    }

    SECTION("clear") {
        fixed_string_writer<32> sw;

        sw.append("test");
        sw.clear();

        CHECK(sw.empty());
        CHECK(sw.empty());
        CHECK(sw.c_str() == ""_sv);
    }

    SECTION("overflow write") {
        fixed_string_writer<32> sw;

        sw.append("initial text");
        sw.append("more text");
        sw.append("yet more text");
        sw.append("and some more text");

        CHECK(sw.size() == sw.capacity());
        CHECK(sw.c_str() == "initial textmore textyet more t"_sv);
    }
}
