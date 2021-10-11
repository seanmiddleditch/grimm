// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/zstring_view.h"

#include <catch2/catch.hpp>
#include <cstring>
#include <ostream>

TEST_CASE("potato.spud.zstring_view", "[potato][spud]") {
    using namespace up;

    SECTION("default initialization") {
        zstring_view sv;

        CHECK(sv.empty());
        CHECK(sv.empty());
    }

    SECTION("std::string initialization") {
        std::string s = "this is a test";
        zstring_view sv(s.c_str());

        CHECK_FALSE(sv.empty());
        CHECK(sv.size() == s.size());
        CHECK(sv == s.c_str());
    }

    SECTION("literal initialization") {
        zstring_view sv = "this is a test";

        CHECK_FALSE(sv.empty());
        CHECK(sv.size() == 14);
        CHECK(sv == "this is a test");
    }

    SECTION("C string initialization") {
        char const* cs = "this is a test";
        zstring_view sv = cs;

        CHECK_FALSE(sv.empty());
        CHECK(sv.size() == std::strlen(cs));
        CHECK(sv == cs);
    }

    SECTION("slicing") {
        zstring_view sv = "this is a test";

        CHECK(sv.first(7) == "this is");
        CHECK(sv.substr(8) == "a test");
        CHECK(sv.substr(8, 1) == "a");

        CHECK(sv.front() == 't');
    }

    SECTION("searching") {
        zstring_view sv = "this is a test";

        CHECK(sv.find('a') == 8);
        CHECK(sv.find('z') == zstring_view::npos);

        CHECK(sv.find_first_of("ea") == 8);
        CHECK(sv.find_first_of("ez") == 11);
        CHECK(sv.find_first_of("fz") == zstring_view::npos);

        CHECK(sv.find_last_of("ih") == 5);
        CHECK(sv.find_last_of("zh") == 1);
        CHECK(sv.find_last_of("fz") == zstring_view::npos);
    }
}
