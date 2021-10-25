// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/query.h"
#include "potato/game/world.h"
#include "potato/schema/test_components_schema.h"

#include <catch2/catch.hpp>

CATCH_REGISTER_ENUM(up::EntityId);

TEST_CASE("potato.ecs.World", "[potato][ecs]") {
    using namespace up;
    using namespace up::components;

    SECTION("directly access componens") {
        World world;
        world.registerComponent<Test1>();
        world.registerComponent<Second>();
        world.registerComponent<Another>();
        world.registerComponent<Counter>();

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        auto id = world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});

        auto* test = world.getComponentSlow<Second>(id);
        REQUIRE(test != nullptr);
        CHECK(test->a == 'h');
        CHECK(test->b == -2.f);
    }

    SECTION("create and delete entities") {
        World world;
        world.registerComponent<Test1>();
        world.registerComponent<Second>();
        world.registerComponent<Another>();
        world.registerComponent<Counter>();

        // create some dummy entities
        //
        EntityId foo = world.createEntity(Test1{'a'});
        world.createEntity(Test1{'b'});
        EntityId bar = world.createEntity(Test1{'c'});
        world.createEntity(Test1{'d'});
        EntityId last = world.createEntity(Test1{'e'});

        auto* fooTest = world.getComponentSlow<Test1>(foo);
        REQUIRE(fooTest != nullptr);

        // delete some entities (not the last one!)
        //
        world.deleteEntity(foo);
        world.deleteEntity(bar);

        // ensure deleted entities are gone
        //
        CHECK(world.getComponentSlow<Test1>(foo) == nullptr);
        CHECK(world.getComponentSlow<Test1>(bar) == nullptr);

        // overwrite emptied locations
        //
        world.createEntity(Test1{'x'});
        world.createEntity(Test1{'x'});

        // ensure that the first deleted entity was overwritten properly
        //
        CHECK(fooTest->a == 'e');

        // ensure that the last entity was moved properly
        //
        CHECK(world.getComponentSlow<Test1>(last)->a == 'e');
    }

    SECTION("remove components") {
        bool found = false;
        World world;
        world.registerComponent<Test1>();
        world.registerComponent<Second>();
        world.registerComponent<Another>();
        world.registerComponent<Counter>();

        Query<Test1> queryTest1;
        Query<Second> querySecond;

        EntityId id = world.createEntity(Test1{}, Second{});

        querySecond.select(world, [&found](EntityId, Second&) { found = true; });
        CHECK(found);

        world.removeComponent<Second>(id);

        found = false;
        querySecond.select(world, [&found](EntityId, Second&) { found = true; });
        CHECK_FALSE(found);

        found = false;
        queryTest1.select(world, [&found](EntityId, Test1&) { found = true; });
        CHECK(found);
    }

    SECTION("add component") {
        bool found = false;
        World world;
        world.registerComponent<Test1>();
        world.registerComponent<Second>();
        world.registerComponent<Another>();
        world.registerComponent<Counter>();

        Query<Test1> queryTest1;
        Query<Second> querySecond;

        EntityId id = world.createEntity(Test1{});

        querySecond.select(world, [&found](EntityId, Second&) { found = true; });
        CHECK_FALSE(found);

        world.addComponent(id, Second{});

        found = false;
        querySecond.select(world, [&found](EntityId, Second&) { found = true; });
        CHECK(found);

        found = false;
        queryTest1.select(world, [&found](EntityId, Test1&) { found = true; });
        CHECK(found);
    }
}
