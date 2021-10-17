// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "demo_system.h"
#include "space.h"
#include "world.h"

#include "potato/audio/audio_engine.h"

#include <glm/gtx/rotate_vector.hpp>

up::game::DemoSystem::DemoSystem(Space& space, AudioEngine& audioEngine) : System(space), _audioEngine(audioEngine) { }

void up::game::DemoSystem::start() {
    space().world().createQuery(_waveQuery);
    space().world().createQuery(_orbitQuery);
    space().world().createQuery(_spinQuery);
    space().world().createQuery(_dingQuery);
}

void up::game::DemoSystem::update(float deltaTime) {
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
