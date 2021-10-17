// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene.h"
#include "components_schema.h"

#include "potato/audio/audio_engine.h"
#include "potato/game/query.h"
#include "potato/game/world.h"
#include "potato/render/mesh.h"
#include "potato/runtime/json.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

up::Scene::Scene(Universe& universe, AudioEngine& audioEngine)
    : Space(universe.createWorld())
    , _audioEngine(audioEngine)
    , _universe(universe)
    , _waveQuery{universe.createQuery<components::Transform, components::Wave>()}
    , _orbitQuery{universe.createQuery<components::Transform>()}
    , _spinQuery{universe.createQuery<components::Transform, components::Spin>()}
    , _dingQuery{universe.createQuery<components::Ding>()} {}

up::Scene::~Scene() = default;

void up::Scene::update(float frameTime) {
    if (_playing) {
        _waveQuery.select(world(), [&](EntityId, components::Transform& trans, components::Wave& wave) {
            wave.offset += frameTime * .2f;
            trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
        });

        _orbitQuery.select(world(), [&](EntityId, components::Transform& trans) {
            trans.position = glm::rotateY(trans.position, frameTime);
        });

        _spinQuery.select(world(), [&](EntityId, components::Transform& trans, components::Spin const& spin) {
            trans.rotation = glm::angleAxis(spin.radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
        });

        _dingQuery.select(world(), [&, this](EntityId, components::Ding& ding) {
            ding.time += frameTime;
            if (ding.time > ding.period) {
                ding.time -= ding.period;
                _audioEngine.play(ding.sound.asset());
            }
        });
    }

    Space::update(frameTime);
}

auto up::Scene::load(Stream file) -> bool {
    if (auto [rs, doc] = readJson(file); rs == IOResult::Success) {
        return false;
    }

    return false;
}

void up::Scene::save(Stream file) {
    auto doc = nlohmann::json::object();
}
