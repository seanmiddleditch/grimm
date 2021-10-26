// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"

#include <catch2/catch.hpp>

CATCH_REGISTER_ENUM(up::EntityId);

namespace components {
    struct Test1 {
        char a;
    };

    struct Second {
        float b;
        char a;
    };

    struct Another {
        double a;
        float b;
    };

    struct Counter {
        int value;
    };
} // namespace components

TEST_CASE("potato.ecs.EntityManager", "[potato][ecs]") {
    using namespace up;
    using namespace components;

    SECTION("directly access componens") {
        EntityManager entities;
        entities.registerComponent<Test1>();
        entities.registerComponent<Second>();
        entities.registerComponent<Another>();
        entities.registerComponent<Counter>();

        entities.createEntity(Test1{'f'}, Second{7.f, 'g'});
        entities.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        auto id = entities.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});

        auto* test = entities.getComponentSlow<Second>(id);
        REQUIRE(test != nullptr);
        CHECK(test->a == 'h');
        CHECK(test->b == -2.f);
    }

    SECTION("create and delete entities") {
        EntityManager entities;
        entities.registerComponent<Test1>();
        entities.registerComponent<Second>();
        entities.registerComponent<Another>();
        entities.registerComponent<Counter>();

        // create some dummy entities
        //
        EntityId foo = entities.createEntity(Test1{'a'});
        entities.createEntity(Test1{'b'});
        EntityId bar = entities.createEntity(Test1{'c'});
        entities.createEntity(Test1{'d'});
        EntityId last = entities.createEntity(Test1{'e'});

        auto* fooTest = entities.getComponentSlow<Test1>(foo);
        REQUIRE(fooTest != nullptr);

        // delete some entities (not the last one!)
        //
        entities.deleteEntity(foo);
        entities.deleteEntity(bar);

        // ensure deleted entities are gone
        //
        CHECK(entities.getComponentSlow<Test1>(foo) == nullptr);
        CHECK(entities.getComponentSlow<Test1>(bar) == nullptr);

        // overwrite emptied locations
        //
        entities.createEntity(Test1{'x'});
        entities.createEntity(Test1{'x'});

        // ensure that the first deleted entity was overwritten properly
        //
        CHECK(fooTest->a == 'e');

        // ensure that the last entity was moved properly
        //
        CHECK(entities.getComponentSlow<Test1>(last)->a == 'e');
    }

    SECTION("remove components") {
        bool found = false;
        EntityManager entities;
        entities.registerComponent<Test1>();
        entities.registerComponent<Second>();
        entities.registerComponent<Another>();
        entities.registerComponent<Counter>();

        EntityId id = entities.createEntity(Test1{}, Second{});

        entities.select<Second>([&found](EntityId, Second&) { found = true; });
        CHECK(found);

        entities.removeComponent<Second>(id);

        found = false;
        entities.select<Second>([&found](EntityId, Second&) { found = true; });
        CHECK_FALSE(found);

        found = false;
        entities.select<Test1>([&found](EntityId, Test1&) { found = true; });
        CHECK(found);
    }

    SECTION("add component") {
        bool found = false;
        EntityManager entities;
        entities.registerComponent<Test1>();
        entities.registerComponent<Second>();
        entities.registerComponent<Another>();
        entities.registerComponent<Counter>();

        EntityId id = entities.createEntity(Test1{});

        entities.select<Second>([&found](EntityId, Second&) { found = true; });
        CHECK_FALSE(found);

        entities.addComponent(id, Second{});

        found = false;
        entities.select<Second>([&found](EntityId, Second&) { found = true; });
        CHECK(found);

        found = false;
        entities.select<Test1>([&found](EntityId, Test1&) { found = true; });
        CHECK(found);
    }
}
