#include "potato/runtime/uuid.h"
#include <doctest/doctest.h>
#include <array>

DOCTEST_TEST_SUITE("[potato][runtime] up::UUID") {
    using namespace up;

    DOCTEST_TEST_CASE("basic uuid") {
        UUID zero = UUID();
        DOCTEST_CHECK(zero == UUID::zero());
        DOCTEST_CHECK_EQ(UUID::toString(zero).c_str(), "00000000-0000-0000-0000-000000000000");

        UUID a = UUID::generate();
        DOCTEST_CHECK(a.isValid());

        string str = UUID::toString(a);
        UUID b = UUID::fromString(str);
        DOCTEST_CHECK(b == a);

        const UUID c = UUID::fromString("00000000-0000-0000-0000-000000000000");
        DOCTEST_CHECK(c == UUID::zero());

        // check invalid length of string
        const UUID d = UUID::fromString("9554084e-4100-4098-b470-2125f5eed13");
        DOCTEST_CHECK(d == UUID::zero());
        const UUID e = UUID::fromString("9554084e-4100-4098-b470-2125f5eed133f");
        DOCTEST_CHECK(e == UUID::zero());
        const UUID f = UUID::fromString("955408-4e4100-4098-b470-2125f5eed133f");
        DOCTEST_CHECK(f == UUID::zero());
        const UUID g = UUID::fromString("9554084e-41K0-4098-PODR-25f5eed133fN");
        DOCTEST_CHECK(g == UUID::zero());

    }
}
