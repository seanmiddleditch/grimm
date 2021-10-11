// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "reflex_test_schema.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.reflex.Schema", "[potato][reflex]") {
    using namespace up;
    using namespace up::schema;

    SECTION("enumToString") {
        CHECK("First"_sv == reflex::enumToString(TestEnum::First));
        CHECK("Third"_sv == reflex::enumToString(TestEnum::Third));
    }

    SECTION("enumToValue") {
        CHECK(TestEnum::First == reflex::enumToValue<TestEnum>("First"_sv));
        CHECK(TestEnum::Third == reflex::enumToValue<TestEnum>("Third"_sv));
    }

    SECTION("getSchema") {
        reflex::Schema const& schema = reflex::getSchema<TestStruct>();
        CHECK(schema.name == "TestStruct"_sv);
        CHECK(schema.primitive == reflex::SchemaPrimitive::Object);
        REQUIRE(schema.fields.size() == 1);
        CHECK(schema.fields[0].name == "test"_sv);
    }
}
