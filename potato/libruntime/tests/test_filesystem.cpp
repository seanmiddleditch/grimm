// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <catch2/catch.hpp>
#include <algorithm>
#include <iostream>
#include <string>

TEST_CASE("potato.runtime.filesystem", "[potato][runtime]") {
    using namespace up;
    using namespace up::fs;

    SECTION("fileExists") {
        CHECK(fileExists("test.txt"));
        CHECK_FALSE(fileExists("foobar.txt"));

        CHECK_FALSE(fileExists("parent"));
    }

    SECTION("directoryExists") {
        CHECK(directoryExists("parent"));
        CHECK(directoryExists("parent/child"));

        CHECK_FALSE(directoryExists("parent/child/grandchild"));
        CHECK_FALSE(directoryExists("test.txt"));
    }

    SECTION("openRead") {
        auto inFile = openRead("test.txt", OpenMode::Text);
        REQUIRE(inFile.isOpen());

        byte buffer[1024];
        span<byte> bspan(buffer);
        inFile.read(bspan);
        string_view text(bspan.as_chars().data(), bspan.size());

        CHECK(text.first(15) == "This is a test."_sv);
    }

    SECTION("enumerate") {
        vector<string> const expected{"parent"_s, "parent/child"_s, "parent/child/hello.txt"_s, "test.txt"_s};

        vector<string> entries;

        auto cb = [&entries](auto const& item, int) {
            entries.push_back(item.path);
            return recurse;
        };
        CHECK(enumerate(".", cb) == next);

        REQUIRE(entries.size() == expected.size());

        // platforms aren't necessarily going to return entries in the same order
        std::sort(entries.begin(), entries.end());

        for (typename decltype(entries)::size_type i = 0, e = entries.size(); i != e; ++i) {
            CHECK(entries[i] == expected[i]);
        }
    }

    SECTION("stat") {
        auto const [rs, stat] = fileStat("test.txt");
        REQUIRE(rs == IOResult::Success);
        CHECK(stat.type == FileType::Regular);

        // note: can't test size (Windows/UNIX line endings!) or mtime (git)
    }

    // TODO:
    // Figure out how to reliably test "write" operations on the native backend.
    // - [ ] test openWrite
    // - [ ] test createDirectories
    // - [ ] test copyFile
}
