// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "components_schema.h"
#include "query.h"
#include "system.h"

namespace up::game {
    class TransformSystem final : public System {
    public:
        using System::System;

        void start() override;
        void stop() override { }

        void update(float) override;
        void render(Renderer&) override { }

    private:
        Query<components::Transform> _transformQuery;
    };
} // namespace up::game
