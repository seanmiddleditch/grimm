// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/nameof.h"

#include <catch2/catch.hpp>
#include <iostream>

static_assert(up::nameof<int>() == "int");

template <typename T>
struct as_template {};

TEST_CASE("potato.spud.nameof", "[potato][spud]") {
    using namespace up;

    SECTION("builtin types") {
        CHECK("int" == nameof<int>());
        CHECK("float" == nameof<float>());
        CHECK("char" == nameof<char>());
    }

    SECTION("class types") {
#if defined(UP_COMPILER_MICROSOFT)
        CHECK("class up::string_view" == nameof<string_view>());
#else
        CHECK("up::string_view" == nameof<string_view>());
#endif
    }

    SECTION("template types") {
#if defined(UP_COMPILER_MICROSOFT)
        CHECK("struct as_template<int>" == nameof<as_template<int>>());
#else
        CHECK("as_template<int>" == nameof<as_template<int>>());
#endif
    }
}
