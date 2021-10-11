#include "potato/spud/delegate_ref.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.delegate_ref", "[potato][spud]") {
    using namespace up;

    SECTION("lambda delegate_ref") {
        int (*f)(int) = [](int i) {
            return i * 2;
        };
        delegate_ref d = f;

        CHECK(d(0) == 0);
        CHECK(d(-1) == -2);
        CHECK(d(10) == 20);
    }

    SECTION("delegate_ref reassignment") {
        int i1 = 2;
        auto f1 = [&i1](int i) {
            return i1 += i;
        };
        static_assert(is_invocable_v<decltype(f1), int>);

        int i2 = 2;
        auto f2 = [&i2](int i) {
            return i2 *= i;
        };

        delegate_ref<int(int)> d(f1);
        d(2);
        CHECK(i1 == 4);

        d = f2;
        d(4);
        CHECK(i2 == 8);
    }
}
