// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/sequence.h"

#include <catch2/catch.hpp>

namespace {
    enum WeakEnum { A, B, C, Max };
    enum class StrongEnum { A, B, C, Max };
} // namespace

TEST_CASE("potato.spud.sequence", "[potato][spud]") {
    using namespace up;

    SECTION("sequence range") {
        CHECK(sequence(10).size() == 10);
        CHECK_FALSE(sequence(10).empty());

        if (auto seq = sequence(3); !seq.empty()) {
            auto it = seq.begin();
            CHECK(*(it++) == 0);
            CHECK(*(it++) == 1);
            CHECK(*(it++) == 2);

            CHECK(it == seq.end());
        }
    }

    SECTION("sequence with initial value") {
        CHECK(sequence(5, 10).size() == 5);
        CHECK_FALSE(sequence(5, 10).empty());

        CHECK(sequence(10, 10).empty());
        CHECK(sequence(10, 10).empty());

        if (auto seq = sequence(5, 10); !seq.empty()) {
            auto it = seq.begin();
            CHECK(*(it++) == 5);
            CHECK(*(it++) == 6);
            CHECK(*(it++) == 7);
            CHECK(*(it++) == 8);
            CHECK(*(it++) == 9);

            CHECK(it == seq.end());
        }
    }

    SECTION("sequence with weak enum") {
        auto seq = sequence<WeakEnum>(A, Max);

        CHECK(seq.size() == 3);

        auto it = seq.begin();
        CHECK(*(it++) == A);
        CHECK(*(it++) == B);
        CHECK(*(it++) == C);

        CHECK(it == seq.end());
    }

    SECTION("sequence with strong enum") {
        auto seq = sequence<StrongEnum>(StrongEnum::A, StrongEnum::Max);

        CHECK(seq.size() == 3);

        auto it = seq.begin();
        CHECK(*(it++) == StrongEnum::A);
        CHECK(*(it++) == StrongEnum::B);
        CHECK(*(it++) == StrongEnum::C);

        CHECK(it == seq.end());
    }
}
