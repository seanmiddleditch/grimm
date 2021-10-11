// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "test_components_schema.h"

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
#include "potato/ecs/world.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.ecs.Query", "[potato][ecs]") {
    using namespace up;
    using namespace up::components;

    Universe universe;

    universe.registerComponent<Test1>("Test1");
    universe.registerComponent<Second>("Second");
    universe.registerComponent<Another>("Another");

    SECTION("selecting chunks") {
        auto world = universe.createWorld();

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        auto query = universe.createQuery<Second>();
        query.selectChunks(world, [&](size_t count, EntityId const*, Second* second) {
            ++invokeCount;
            entityCount += count;

            for (size_t index = 0; index != count; ++index) {
                weight += second[index].b;
            }
        });

        // Only two archetypes should have matches
        CHECK(invokeCount == 2);

        // Three total entities between the two archetypes should exist that match
        CHECK(entityCount == 3);

        // Ensure we're storing/retrieving correct values
        CHECK(weight == 14.f);
    }

    SECTION("selecting entities") {
        auto world = universe.createWorld();

        world.createEntity(Second{1.f, 'g'});
        world.createEntity(Second{2.f, 'g'});
        world.createEntity(Second{3.f, 'g'});
        world.createEntity(Second{4.f, 'g'});

        auto query = universe.createQuery<Second>();
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
