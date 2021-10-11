// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/resource_manifest.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.runtime.ResourceManifest", "[potato][runtime]") {
    using namespace up;

    SECTION("parse") {
        string_view input =
            "# Comment\n"
            "\n" // blank
            ".meta=value\n"
            ".key=var\n"
            ":UUID|LOGICAL_ID|LOGICAL_NAME|CONTENT_TYPE|CONTENT_HASH|DEBUG_NAME\n"
            "D8E02451-6D48-49F6-A2D3-9281379CB75A|0|0|nil|DEAD|zero\n"
            "F2D1B621-9A00-4263-9786-80073F493796|1|0|bin|C0DE|one\n"
            ""_sv;
        ResourceManifest manifest;
        auto parseResult = ResourceManifest::parseManifest(input, manifest);
        REQUIRE(parseResult);

        CHECK(manifest.size() == 2);

        CHECK(manifest.records().front().uuid == UUID::fromString("D8E02451-6D48-49F6-A2D3-9281379CB75A"));
        CHECK(manifest.records().front().hash == 0xDEAD);
        CHECK(string_view{manifest.records().front().filename} == "zero"_sv);

        CHECK(manifest.records().back().uuid == UUID::fromString("F2D1B621-9A00-4263-9786-80073F493796"));
        CHECK(manifest.records().back().hash == 0xC0DE);
        CHECK(string_view{manifest.records().back().filename} == "one"_sv);
    }
}
