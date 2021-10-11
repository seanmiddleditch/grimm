// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.vector", "[potato][spud]") {
    using namespace up;

    SECTION("vector default initialization") {
        vector<int> vec;

        CHECK(vec.empty());
        CHECK(vec.capacity() == 0);
        CHECK(vec.empty());
    }

    SECTION("vector initializer list") {
        vector vec{1, 2, 3, 4};

        REQUIRE(vec.size() == 4);
        CHECK(vec.capacity() == 4);
        CHECK_FALSE(vec.empty());

        CHECK(vec.front() == 1);
        CHECK(vec.back() == 4);
    }

    SECTION("vector push_back") {
        vector<int> vec;

        for (int i = 1; i <= 10'000; ++i) {
            vec.push_back(i * i);
        }

        CHECK(vec.size() == 10'000);
        CHECK(vec.capacity() >= 10'000);
        CHECK_FALSE(vec.empty());

        REQUIRE(vec.begin() + 10'000 == vec.end());
        CHECK(&*vec.begin() == vec.data());

        CHECK(vec.front() == 1);
        CHECK(*vec.begin() == 1);
        CHECK(*vec.data() == 1);

        CHECK(vec.back() == 10'000 * 10'000);
    }
    SECTION("vector insert") {
        vector<int> vec;

        for (int i = 1; i <= 10'000; ++i) {
            vec.insert(vec.begin(), i);
        }

        CHECK(vec.size() == 10'000);
        CHECK_FALSE(vec.empty());

        CHECK(vec.back() == 1);
        CHECK(vec.front() == 10'000);
    }

    SECTION("vector resize") {
        vector vec{1, 2, 3, 4};

        vec.resize(5);

        CHECK(vec.size() == 5);

        vec.resize(6, 7);

        CHECK(vec.size() == 6);
        CHECK(vec.front() == 1);
        CHECK(vec.back() == 7);
    }

    SECTION("vector erase") {
        vector vec{1, 2, 3, 4};

        vec.erase(vec.begin());

        CHECK(vec.size() == 3);
        CHECK(vec.front() == 2);
        CHECK(vec.back() == 4);

        vec.erase(vec.begin() + 1, vec.end());

        CHECK(vec.size() == 1);
        CHECK(vec.front() == 2);
    }

    SECTION("vector<string>") { vector<string> vec1{"first"_s, "second"_s, "third"_s, "fourth"_s}; }
}
