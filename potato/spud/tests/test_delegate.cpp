#include "potato/spud/delegate.h"

#include <catch2/catch.hpp>

namespace {
    struct Test {
        int& r;

        int next() { return ++r; }
        int add(int i) const { return r + i; }
    };
} // namespace

TEST_CASE("potato.spud.delegate", "[potato][spud]") {
    using namespace up;

    SECTION("empty delegate") {
        delegate<int()> d;

        CHECK(d.empty());

        d.reset();
    }

    SECTION("lambda delegate") {
        delegate d = +[](int i) {
            return i * 2;
        };

        CHECK_FALSE(d.empty());
        CHECK(d(0) == 0);
        CHECK(d(-1) == -2);
        CHECK(d(10) == 20);

        d.reset();

        CHECK(d.empty());
    }

    SECTION("object delegate") {
        int i = 1;
        Test test{i};

        CHECK(delegate(test, &Test::next)() == 2);
        CHECK(delegate(test, &Test::next)() == 3);
        CHECK(delegate(test, &Test::add)(4) == 7);
    }

    SECTION("void delegate") {
        int i = 0;
        delegate<void()> d = [&i] {
            i = 1;
        };

        d();
        CHECK(i == 1);
    }

    SECTION("delegate reassignment") {
        int i1 = 1;
        Test t1{i1};

        int i2 = 1;
        Test t2{i2};

        delegate<int()> d;
        d = delegate(&t1, &Test::next);
        CHECK_FALSE(d.empty());
        d();
        CHECK(i1 == 2);

        d = delegate(&t2, &Test::next);
        CHECK_FALSE(d.empty());
        d();
        CHECK(i2 == 2);
    }
}
