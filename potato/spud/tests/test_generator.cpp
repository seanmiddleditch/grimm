// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/generator.h"

#include <catch2/catch.hpp>
#include <sstream>
#include <string_view>

TEST_CASE("potato.spud.generator", "[potato][spud]") {
    using namespace up;

    struct Count {
        up::generator<int> operator()(int last) {
            for (int i = 0; i != last; ++i) {
                co_yield i;
            }
        }
    };

    struct Strings {
        up::generator<std::string_view> operator()(int last) {
            std::ostringstream buf;
            std::string str;
            for (int i = 0; i != last; ++i) {
                buf << '[' << i << ']';
                str = buf.str();
                buf.str("");

                std::string_view sv = str;
                co_yield sv;
            }
        }
    };

    SECTION("generator") {
        std::ostringstream buf;
        for (int i : Count{}(7)) {
            buf << i;
        }
        CHECK(buf.str() == "0123456");

        buf.str("");
        for (std::string_view i : Strings{}(3)) {
            buf << i;
        }
        CHECK(buf.str() == "[0][1][2]");
    }
}
