// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"
#include "potato/game/query.h"
#include "potato/schema/test_components_schema.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.ecs.Query", "[potato][ecs]") {
    using namespace up;
    using namespace up::components;

    SECTION("selecting entities") {
        EntityManager entities;
        entities.registerComponent<Test1>();
        entities.registerComponent<Second>();
        entities.registerComponent<Another>();

        entities.createEntity(Second{1.f, 'g'});
        entities.createEntity(Second{2.f, 'g'});
        entities.createEntity(Second{3.f, 'g'});
        entities.createEntity(Second{4.f, 'g'});

        Query<Second> query;
        float sum = 0;
        int count = 0;
        query.select(entities, [&](EntityId, Second const& second) {
            ++count;
            sum += second.b;
        });

        CHECK(count == 4);
        CHECK(sum == 10.0f);
    }
}
