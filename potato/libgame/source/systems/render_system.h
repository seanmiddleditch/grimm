// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "components_schema.h"
#include "query.h"
#include "system.h"

namespace up::game {
    class RenderSystem final : public System {
    public:
        using System::System;

        void start() override;
        void stop() override;

        void update(float deltaTime) override;
        void render(RenderContext& ctx) override;

    private:
        Query<components::Mesh, components::Transform> _meshQuery;
    };
} // namespace up::game
