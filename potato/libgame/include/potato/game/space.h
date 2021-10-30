// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "ecs_system.h"
#include "world.h"

#include "potato/spud/box.h"
#include "potato/spud/vector.h"

namespace up {
    class Renderer;

    class Space {
    public:
        UP_GAME_API Space();
        UP_GAME_API virtual ~Space();

        UP_GAME_API virtual void update(float deltaTime);
        UP_GAME_API virtual void render(Renderer& renderer);

        void addSystem(box<System> system);

    private:
        vector<box<System>> _systems;
    };
} // namespace up
