// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/callstack.h"

#include <catch2/catch.hpp>
#include <array>

TEST_CASE("potato.runtime.callstack", "[potato][runtime]") {
    using namespace up;
    using namespace up::callstack;

    SECTION("read callstack") {
        std::array<uintptr, 1> addresses = {};

        auto stack = readTrace(span{addresses.data(), addresses.size()});
        CHECK_FALSE(stack.empty());
    }
}
