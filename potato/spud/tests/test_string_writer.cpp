// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"
#include "potato/spud/string_writer.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.spud.string_writer", "[potato][spud]") {
    using namespace up;

    SECTION("default initialization") {
        string_writer sw;

        CHECK(sw.empty());
        CHECK(sw.empty());
        CHECK(sw.c_str() == ""_s);
    }

    SECTION("reserve initialization") {
        string_writer sw(64);

        CHECK(sw.empty());
        CHECK(sw.empty());
        CHECK(sw.c_str() == ""_s);
    }

    SECTION("write") {
        string_writer sw;

        sw.append("hello"_s);
        sw.append(',');
        sw.append(' ');
        sw.append("world"_s);

        CHECK_FALSE(sw.empty());
        CHECK(sw.size() == 12);
        CHECK(sw.c_str() == "hello, world"_sv);
    }

    SECTION("clear") {
        string_writer sw;

        sw.append("test"_s);
        sw.clear();

        CHECK(sw.empty());
        CHECK(sw.empty());
        CHECK(sw.c_str() == ""_s);
    }

    SECTION("request and commit") {
        string_writer sw;

        sw.append("initial text"_s);

        auto mem = sw.acquire(32);

        CHECK(mem.size() >= 32);
        CHECK((void*)mem.data() == (void*)(sw.data() + sw.size()));

        std::memcpy(mem.data(), "data", 4);

        sw.commit(mem.first(4));

        sw.append("footer"_s);

        CHECK(sw.c_str() == "initial textdatafooter"_s);
    }

    SECTION("resize") {
        string_writer sw;

        sw.append("initial text"_s);

        sw.resize(4);
        CHECK(sw.c_str() == "init"_s);

        sw.resize(6);
        CHECK(sw.c_str() == "init  "_s);

        sw.resize(8, 'x');
        CHECK(sw.c_str() == "init  xx"_s);

        sw.append("yy"_s);
        CHECK(sw.c_str() == "init  xxyy"_s);
    }

    SECTION("to_string") {
        string_writer sw;

        sw.append("some text here"_s);

        string s = sw.to_string();

        CHECK(sw.c_str() == "some text here"_s);
        CHECK(s.c_str() == "some text here"_s);

        s = std::move(sw).to_string();

        // NOLINTNEXTLINE(bugprone-use-after-move)
        CHECK(sw.empty());
        CHECK(s.c_str() == "some text here"_s);
    }
}
