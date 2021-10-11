// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "test_components_schema.h"

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
#include "potato/ecs/world.h"

#include <catch2/catch.hpp>

CATCH_REGISTER_ENUM(up::EntityId);

TEST_CASE("potato.ecs.World", "[potato][ecs]") {
    using namespace up;
    using namespace up::components;

    Universe universe;

    universe.registerComponent<Test1>("Test1");
    universe.registerComponent<Second>("Second");
    universe.registerComponent<Another>("Another");
    universe.registerComponent<Counter>("Counter");

    SECTION("directly access componens") {
        auto world = universe.createWorld();

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        auto id = world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});

        auto* test = world.getComponentSlow<Second>(id);
        REQUIRE(test != nullptr);
        CHECK(test->a == 'h');
        CHECK(test->b == -2.f);
    }

    SECTION("allocate EntityId") {
        auto world = universe.createWorld();

        EntityId first = world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        EntityId second = world.createEntity(Test1{'h'}, Second{-1.f, 'i'});

        auto query = universe.createQuery<Test1>();
        query.selectChunks(world, [&](size_t count, EntityId const* entities, Test1*) {
            REQUIRE(count == 2);
            CHECK(entities[0] == first);
            CHECK(entities[1] == second);
        });
    }

    SECTION("iterate chunks") {
        constexpr int count = 100000;
        auto world = universe.createWorld();

        uint64 expectedSum = 0;
        for (int i = 0; i != count; ++i) {
            expectedSum += i;
            world.createEntity(Counter{i});
        }

        size_t chunks = 0;
        size_t total = 0;
        uint64 sum = 0;

        auto query = universe.createQuery<Counter>();
        query.selectChunks(world, [&](size_t count, EntityId const*, Counter* counters) {
            ++chunks;
            total += count;
            for (size_t i = 0; i != count; ++i) {
                sum += counters[i].value;
            }
        });

        CHECK(chunks != 0);
        CHECK(total == count);
        CHECK(sum == expectedSum);
    }

    SECTION("create and delete entities") {
        auto world = universe.createWorld();

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
        auto world = universe.createWorld();
        auto queryTest1 = universe.createQuery<Test1>();
        auto querySecond = universe.createQuery<Second>();

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

        found = false;
        world.interrogateEntityUnsafe(id, [&found](EntityId, ArchetypeId, reflex::TypeInfo const*, void*) {
            found = true;
        });
        CHECK(found);
    }

    SECTION("add component") {
        bool found = false;
        auto world = universe.createWorld();
        auto queryTest1 = universe.createQuery<Test1>();
        auto querySecond = universe.createQuery<Second>();

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

    SECTION("iterrogate entities") {
        auto world = universe.createWorld();

        auto id = world.createEntity(Test1{'f'}, Another{1.0, 2.f}, Second{7.f, 'g'});

        auto success = world.interrogateEntityUnsafe(id, [&](auto entity, auto archetype, auto component, auto data) {
            if (component->hash == reflex::TypeHolder<Test1>::get().hash) {
                CHECK(static_cast<Test1 const*>(data)->a == 'f');
            }
            else if (component->hash == reflex::TypeHolder<Another>::get().hash) {
                CHECK(static_cast<Another const*>(data)->a == 1.0);
            }
            else if (component->hash == reflex::TypeHolder<Second>::get().hash) {
                CHECK(static_cast<Second const*>(data)->b == 7.f);
            }
            else {
                CHECK_FALSE("unknown component");
            }
        });
        CHECK(success);
    }
}
