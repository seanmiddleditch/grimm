// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/audio/audio_engine.h"
#include "potato/game/components/demo_components.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/rotate_vector.hpp>

namespace up {
    namespace {
        class DemoSystem final : public System {
        public:
            DemoSystem(Space& space, AudioEngine& audioEngine);

            void update(float) override;

        private:
            AudioEngine& _audioEngine;
        };
    } // namespace

    void registerDemoSystem(Space& space, AudioEngine& audioEngine) { space.addSystem<DemoSystem>(audioEngine); }

    DemoSystem::DemoSystem(Space& space, AudioEngine& audioEngine) : System(space), _audioEngine(audioEngine) { }

    void DemoSystem::update(float deltaTime) {
        space().entities().select<TransformComponent, DemoWaveComponent>(
            [&](EntityId, TransformComponent& trans, DemoWaveComponent& wave) {
                wave.offset += deltaTime * .2f;
                trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
            });

        space().entities().select<TransformComponent, DemoWaveComponent>(
            [&](EntityId, TransformComponent& trans, DemoWaveComponent&) {
            trans.position = glm::rotateY(trans.position, deltaTime);
        });

        space().entities().select<TransformComponent, DemoSpinComponent const>(
            [&](EntityId, TransformComponent& trans, DemoSpinComponent const& spin) {
                trans.rotation = glm::angleAxis(spin.radians * deltaTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
            });

        space().entities().select<DemoDingComponent>([&, this](EntityId, DemoDingComponent& ding) {
            ding.time += deltaTime;
            if (ding.time > ding.period) {
                ding.time -= ding.period;
                _audioEngine.play(ding.sound.asset());
            }
        });
    }
} // namespace up
