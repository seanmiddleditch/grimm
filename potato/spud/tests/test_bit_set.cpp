#include "potato/spud/bit_set.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.bit_set", "[potato][spud]") {
    using namespace up;

    SECTION("empty") {
        bit_set bs;

        CHECK_FALSE(bs.test(0));
    }

    SECTION("set") {
        bit_set bs;

        bs.set(0);
        CHECK(bs.test(0));
        CHECK_FALSE(bs.test(1));
        CHECK(bs.capacity() == 64);

        bs.set(1000);
        CHECK(bs.test(1000));
        CHECK_FALSE(bs.test(999));
        CHECK_FALSE(bs.test(1001));
        CHECK(bs.capacity() == 1024);
    }

    SECTION("reset") {
        bit_set bs;

        bs.set(0);
        bs.reset(0);
        CHECK_FALSE(bs.test(0));
        CHECK_FALSE(bs.test(1));

        bs.set(1000);
        bs.reset(1000);
        CHECK_FALSE(bs.test(1000));
        CHECK_FALSE(bs.test(999));
        CHECK_FALSE(bs.test(1001));
    }

    SECTION("equality") {
        bit_set bs1;
        bit_set bs2;

        bs1.set(0);
        bs1.set(7);
        bs1.set(109);
        bs1.set(555);

        bs2.set(0);
        bs2.set(7);

        CHECK_FALSE(bs1 == bs2);

        bs2.set(109);
        bs2.set(555);

        CHECK(bs1 == bs2);

        bs2.set(909);

        CHECK_FALSE(bs1 == bs2);
    }

    SECTION("has_all") {
        bit_set bs1;
        bit_set bs2;

        bs1.set(0);
        bs1.set(7);
        bs1.set(109);
        bs1.set(555);

        bs2.set(0);
        bs2.set(7);

        CHECK(bs1.has_all(bs2));

        bs2.set(109);
        bs2.set(555);

        CHECK(bs1.has_all(bs2));

        bs2.set(909);

        CHECK_FALSE(bs1.has_all(bs2));
    }
}
