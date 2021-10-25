// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/query.h"
#include "potato/game/world.h"
#include "potato/schema/test_components_schema.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.ecs.Query", "[potato][ecs]") {
    using namespace up;
    using namespace up::components;

    SECTION("selecting entities") {
        World world;
        world.registerComponent<Test1>();
        world.registerComponent<Second>();
        world.registerComponent<Another>();

        world.createEntity(Second{1.f, 'g'});
        world.createEntity(Second{2.f, 'g'});
        world.createEntity(Second{3.f, 'g'});
        world.createEntity(Second{4.f, 'g'});

        Query<Second> query;
        float sum = 0;
        int count = 0;
        query.select(world, [&](EntityId, Second const& second) {
            ++count;
            sum += second.b;
        });

        CHECK(count == 4);
        CHECK(sum == 10.0f);
    }
}
