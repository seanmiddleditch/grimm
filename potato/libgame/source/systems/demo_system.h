// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/query.h"
#include "potato/game/system.h"
#include "potato/schema/components_schema.h"

namespace up {
    class AudioEngine;
}

namespace up::game {
    class DemoSystem final : public System {
    public:
        DemoSystem(Space& space, AudioEngine& audioEngine);

        void start() override;
        void stop() override { }

        void update(float) override;
        void render(RenderContext&) override { }

    private:
        Query<components::Transform, components::Wave> _waveQuery;
        Query<components::Transform> _orbitQuery;
        Query<components::Transform, components::Spin> _spinQuery;
        Query<components::Ding> _dingQuery;
        AudioEngine& _audioEngine;
    };
} // namespace up::game
