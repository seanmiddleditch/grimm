// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "reflex_test_schema.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/json.h"

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("potato.reflex.Serialize", "[potato][reflex]") {
    using namespace up;
    using namespace up::schema;

    SECTION("json encode test") {
        TestStruct s{TestEnum::First};
        nlohmann::json doc;
        CHECK(reflex::encodeToJson(doc, s));
        auto const json = doc.dump();
        CHECK(json == R"({"$schema":"TestStruct","enum":"First"})");
    }

    SECTION("json encode complex") {
        TestComplex comp;
        comp.name = "Frederick"_s;
        comp.values.push_back(42.f);
        comp.values.push_back(-7.f);
        comp.values.push_back(6'000'000'000.f);
        comp.test.test = TestEnum::Second;

        nlohmann::json doc;
        CHECK(reflex::encodeToJson(doc, comp));
        auto const json = doc.dump();
        CHECK(
            json ==
            R"({"$schema":"TestComplex","name":"Frederick","test":{"$schema":"TestStruct","enum":"Second"},"values":[42.0,-7.0,6000000000.0]})");
    }

    SECTION("json decode complex") {
        string_view const json =
            R"({"$schema":"TestComplex","name":"Frederick","values":[42.0,-7.0,6000000000.0],"test":{"$schema":"TestStruct","enum":"Second"}})";

        TestComplex comp;
        CHECK(reflex::decodeFromJson(nlohmann::json::parse(json), comp));

        CHECK(comp.name == "Frederick"_s);
        REQUIRE(comp.values.size() == 3);
        CHECK(comp.values[0] == 42.f);
        CHECK(comp.values[1] == -7.f);
        CHECK(comp.values[2] == 6'000'000'000.f);
        CHECK(comp.test.test == TestEnum::Second);
    }
}
