// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/audio/audio_engine.h"
#include "potato/game/query.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/game/world.h"
#include "potato/schema/components_schema.h"

#include <glm/gtx/rotate_vector.hpp>

namespace {
    using namespace up;

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
} // namespace

namespace up {
    void registerDemoSystem(Space& space, AudioEngine& audioEngine) { space.addSystem<DemoSystem>(audioEngine); }
} // namespace up

DemoSystem::DemoSystem(Space& space, AudioEngine& audioEngine) : System(space), _audioEngine(audioEngine) { }

void DemoSystem::start() {
    space().world().createQuery(_waveQuery);
    space().world().createQuery(_orbitQuery);
    space().world().createQuery(_spinQuery);
    space().world().createQuery(_dingQuery);
}

void DemoSystem::update(float deltaTime) {
    _waveQuery.select(space().world(), [&](EntityId, components::Transform& trans, components::Wave& wave) {
        wave.offset += deltaTime * .2f;
        trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
    });

    _orbitQuery.select(space().world(), [&](EntityId, components::Transform& trans) {
        trans.position = glm::rotateY(trans.position, deltaTime);
    });

    _spinQuery.select(space().world(), [&](EntityId, components::Transform& trans, components::Spin const& spin) {
        trans.rotation = glm::angleAxis(spin.radians * deltaTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
    });

    _dingQuery.select(space().world(), [&, this](EntityId, components::Ding& ding) {
        ding.time += deltaTime;
        if (ding.time > ding.period) {
            ding.time -= ding.period;
            _audioEngine.play(ding.sound.asset());
        }
    });
}
