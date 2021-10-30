// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/rc.h"

#include <catch2/catch.hpp>

namespace {
    struct TestCounted {
        void addRef() noexcept { ++refs; }
        void removeRef() noexcept { --refs; }
        int refs = 1;
    };

    struct TestShared : up::shared<TestShared> {
        TestShared(bool& bind) noexcept : active(bind) { active = true; }
        ~TestShared() noexcept { active = false; }

        bool& active;
    };
} // namespace

TEST_CASE("potato.spud.rc", "[potato][spud]") {
    using namespace up;

    SECTION("empty rc") {
        rc<TestCounted> r;

        CHECK(r.empty());
        CHECK_FALSE(r);
    }

    SECTION("custom rc") {
        TestCounted test;

        rc r(&test);

        CHECK_FALSE(r.empty());
        CHECK(test.refs == 1);

        rc r2 = r;

        CHECK_FALSE(r.empty());
        CHECK_FALSE(r2.empty());
        CHECK(test.refs == 2);

        r2.reset();

        CHECK_FALSE(r.empty());
        CHECK(r2.empty());
        CHECK(test.refs == 1);

        r.reset();

        CHECK(r.empty());
        CHECK(test.refs == 0);
    }

    SECTION("custom rc") {
        bool active = false;
        rc r = new_shared<TestShared>(active);

        CHECK_FALSE(r.empty());
        CHECK(active);

        rc r2 = r;

        CHECK_FALSE(r.empty());
        CHECK_FALSE(r2.empty());
        CHECK(active);

        r2.reset();

        CHECK_FALSE(r.empty());
        CHECK(r2.empty());
        CHECK(active);

        r.reset();

        CHECK(r.empty());
        CHECK_FALSE(active);
    }
}
