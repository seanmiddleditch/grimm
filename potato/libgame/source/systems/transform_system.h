// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/query.h"
#include "potato/game/system.h"
#include "potato/schema/components_schema.h"

namespace up::game {
    class TransformSystem final : public System {
    public:
        using System::System;

        void start() override;
        void stop() override { }

        void update(float) override;
        void render(RenderContext&) override { }

    private:
        Query<components::Transform> _transformQuery;
    };
} // namespace up::game
