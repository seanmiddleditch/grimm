// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/audio/audio_engine.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/schema/components_schema.h"

#include <glm/gtx/rotate_vector.hpp>

namespace {
    using namespace up;

    class DemoSystem final : public System {
    public:
        DemoSystem(Space& space, AudioEngine& audioEngine);

        void start() override { }
        void stop() override { }

        void update(float) override;
        void render(RenderContext&) override { }

    private:
        AudioEngine& _audioEngine;
    };
} // namespace

namespace up {
    void registerDemoSystem(Space& space, AudioEngine& audioEngine) { space.addSystem<DemoSystem>(audioEngine); }
} // namespace up

DemoSystem::DemoSystem(Space& space, AudioEngine& audioEngine) : System(space), _audioEngine(audioEngine) { }

void DemoSystem::update(float deltaTime) {
    space().entities().select<components::Transform, components::Wave>(
        [&](EntityId, components::Transform& trans, components::Wave& wave) {
            wave.offset += deltaTime * .2f;
            trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
        });

    space().entities().select<components::Transform, components::Wave>(
        [&](EntityId, components::Transform& trans, components::Wave&) {
            trans.position = glm::rotateY(trans.position, deltaTime);
        });

    space().entities().select<components::Transform, components::Spin const>(
        [&](EntityId, components::Transform& trans, components::Spin const& spin) {
            trans.rotation = glm::angleAxis(spin.radians * deltaTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
        });

    space().entities().select<components::Ding>([&, this](EntityId, components::Ding& ding) {
        ding.time += deltaTime;
        if (ding.time > ding.period) {
            ding.time -= ding.period;
            _audioEngine.play(ding.sound.asset());
        }
    });
}
