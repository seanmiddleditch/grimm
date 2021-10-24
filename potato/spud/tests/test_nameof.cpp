// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/nameof.h"

#include <catch2/catch.hpp>
#include <iostream>

//static_assert(up::nameof<int>() == "int");

template <typename>
struct wrap {};

template <typename T>
struct as_template { };

TEST_CASE("potato.spud.nameof", "[potato][spud]") {
    using namespace up;

    SECTION("builtin types") {
        CHECK(fixed_string("int") == nameof<int>());
        CHECK(fixed_string("float") == nameof<float>());
        CHECK(fixed_string("char") == nameof<char>());
    }

    SECTION("class types") {
#if defined(UP_COMPILER_MICROSOFT)
        CHECK(fixed_string("class up::string_view") == nameof<string_view>());
#else
        CHECK(fixed_string("up::string_view") == nameof<string_view>());
#endif
    }

    SECTION("template types") {
#if defined(UP_COMPILER_MICROSOFT)
        CHECK(fixed_string("struct as_template<int>") == nameof<as_template<int>>());
#else
        CHECK(fixed_string("as_template<int>") == nameof<as_template<int>>());
#endif
    }
}
