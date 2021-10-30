// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/span.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.span", "[potato][spud]") {
    using namespace up;

    SECTION("span default initialization") {
        span<int> av{};

        CHECK(av.empty());
        CHECK(av.empty());
        CHECK(av.begin() == av.end());
    }

    SECTION("span array") {
        int a[] = {1, 3, 2, 5, 4};
        span av(a);

        CHECK_FALSE(av.empty());
        CHECK(av.size() == 5);
        CHECK(av.data() == a);
        CHECK(av.front() == 1);
        CHECK(av.back() == 4);

        av.pop_front();

        CHECK(av.size() == 4);
        CHECK(av.front() == 3);

        av.pop_back();

        CHECK(av.size() == 3);
        CHECK(av.back() == 5);
    }

    SECTION("span subspan") {
        int a[] = {1, 3, 2, 5, 4};
        span av(a);

        span fv = av.first(3);

        CHECK(fv.size() == 3);
        CHECK(fv.front() == 1);

        span lv = av.last(3);

        CHECK(lv.size() == 3);
        CHECK(lv.front() == 2);

        span sv = av.subspan(2, 2);

        CHECK(sv.size() == 2);
        CHECK(sv.front() == 2);

        span tv = av.subspan(3);

        CHECK(tv.size() == 2);
        CHECK(tv.front() == 5);
    }

    SECTION("span as_bytes") {
        int a[] = {1, 3, 2, 5, 4};
        span av(a);

        span bv = av.as_bytes();

        CHECK(bv.data() == (void*)a);
        CHECK(bv.size() == sizeof(a));
    }
}
