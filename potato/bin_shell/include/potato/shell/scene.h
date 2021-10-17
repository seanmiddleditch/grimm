// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/space.h"
#include "potato/game/universe.h"

namespace up {
    class Universe;

    class Scene : public Space {
    public:
        explicit Scene(Universe& universe);

        void update(float frameTime) override;
    };
} // namespace up
