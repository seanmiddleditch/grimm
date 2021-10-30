// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/path.h"

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("potato.runtime.path_util", "[potato][runtime]") {
    using namespace up;
    using namespace up::path;

    SECTION("extension") {
        CHECK(extension(""_zsv).empty());
        CHECK(extension("/foo/bar.txt"_zsv) == ".txt");
        CHECK(extension(".txt"_zsv) == ".txt");
        CHECK(extension("/foo/bar"_zsv).empty());

        CHECK(extension(""_sv) == "");
        CHECK(extension("/foo/bar.txt"_sv) == ".txt");
        CHECK(extension(".txt"_sv) == ".txt");
        CHECK(extension("/foo/bar"_sv) == "");
    }

    SECTION("changeExtension") {
        CHECK(changeExtension("", ".json") == ".json");
        CHECK(changeExtension("/foo/bar.txt", ".json") == "/foo/bar.json");
        CHECK(changeExtension(".txt", ".json") == ".json");
        CHECK(changeExtension("/foo/bar", ".json") == "/foo/bar.json");
    }

    SECTION("filename") {
        CHECK(filename("") == "");
        CHECK(filename("/foo/bar.txt") == "bar.txt");
        CHECK(filename(".txt") == ".txt");
        CHECK(filename("/foo/bar") == "bar");
    }

    SECTION("filebasename") {
        CHECK(filebasename("") == "");
        CHECK(filebasename("/foo/bar.txt") == "bar");
        CHECK(filebasename(".txt") == "");
        CHECK(filebasename("/foo/bar") == "bar");
    }

    SECTION("parent") {
        CHECK(parent("") == "");
        CHECK(parent("/foo/bar.txt") == "/foo");
        CHECK(parent("bar.txt") == "/");
        CHECK(parent("/foo/bar") == "/foo");
    }

    SECTION("isParentOf") {
        CHECK(isParentOf("/foo", "/foo/bar.txt"));
        CHECK(isParentOf("/foo", "/foo\\bar.txt"));
        CHECK_FALSE(isParentOf("/foo", "/foobar.txt"));
        CHECK_FALSE(isParentOf("/foo", "/foo"));
    }

    SECTION("isNormalized") {
        CHECK(isNormalized("/foo/bar.txt"));
        CHECK(isNormalized("/foo"));
        CHECK(isNormalized("/bar.txt"));

        CHECK(isNormalized(""));
        CHECK(isNormalized("/foo\\bar.txt"));
        CHECK_FALSE(isNormalized("//foo/bar.txt"));
        CHECK(isNormalized("/foo/.bar.txt"));
        CHECK_FALSE(isNormalized("/foo/../bar.txt"));
        CHECK(isNormalized("bar.txt"));
        CHECK_FALSE(isNormalized("/foo/bar/"));
        CHECK(isNormalized("/foo/bar."));
        CHECK(isNormalized("/foo./bar.txt"));
    }

    SECTION("normalize") {
        CHECK(normalize("/foo/bar.txt") == "/foo/bar.txt");
        CHECK(normalize("/foo") == "/foo");
        CHECK(normalize("/bar.txt") == "/bar.txt");

        CHECK(normalize("").empty());
        CHECK(normalize("/foo\\bar.txt") == "/foo/bar.txt");
        CHECK(normalize("//foo/bar.txt") == "/foo/bar.txt");
        CHECK(normalize("/foo/bar.txt") == "/foo/bar.txt");
        CHECK(normalize("bar.txt") == "bar.txt");
        CHECK(normalize("/foo/bar/") == "/foo/bar");

        CHECK(normalize("/foo/../bar/") == "/bar");
        CHECK(normalize("/foo/bar/..") == "/foo");
        CHECK(normalize("/foo/..") == "/");

        CHECK(normalize("/foo/./bar/") == "/foo/bar");
        CHECK(normalize("/foo/bar/.") == "/foo/bar");
        CHECK(normalize("/foo/.") == "/foo");
        CHECK(normalize("./foo") == "/foo");
        CHECK(normalize("./foo/../bar/./../baz/.") == "/baz");
        CHECK(normalize("/foo/../..") == "/");
        CHECK(normalize("/foo/../../bar") == "/bar");
        CHECK(normalize(".") == "/");
        CHECK(normalize("..") == "/");

        CHECK(normalize("..", Separator::Windows) == "\\");
    }

    SECTION("join") {
        CHECK(join("/foo", "bar.txt") == "/foo/bar.txt");
        CHECK(join("/foo", "/bar", "baz.txt") == "/foo//bar/baz.txt");
        CHECK(join("/foo", "", "/bar.txt") == "/foo//bar.txt");
        CHECK(join("", "foo", "/bar.txt") == "foo//bar.txt");
        CHECK(join("foo", "bar", "") == "foo/bar");
        CHECK(join().empty());

        CHECK(join("/foo", "/bar"_sv, "baz.txt"_s) == "/foo//bar/baz.txt");

        CHECK(join(Separator::Windows, "/foo", "/bar"_sv, "baz.txt"_s) == "/foo\\/bar\\baz.txt");
    }
}
