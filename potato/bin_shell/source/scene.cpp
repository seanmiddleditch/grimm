// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene.h"
#include "components_schema.h"

#include "potato/audio/audio_engine.h"
#include "potato/ecs/query.h"
#include "potato/ecs/world.h"
#include "potato/render/mesh.h"
#include "potato/runtime/json.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

up::Scene::Scene(Universe& universe, AudioEngine& audioEngine)
    : _audioEngine(audioEngine)
    , _universe(universe)
    , _world{universe.createWorld()}
    , _waveQuery{universe.createQuery<components::Transform, components::Wave>()}
    , _orbitQuery{universe.createQuery<components::Transform>()}
    , _spinQuery{universe.createQuery<components::Transform, components::Spin>()}
    , _dingQuery{universe.createQuery<components::Ding>()}
    , _transformQuery{universe.createQuery<components::Transform>()}
    , _renderableMeshQuery{universe.createQuery<components::Mesh, components::Transform>()} {}

up::Scene::~Scene() = default;

void up::Scene::tick(float frameTime) {
    if (!_playing) {
        return;
    }

    _waveQuery.select(_world, [&](EntityId, components::Transform& trans, components::Wave& wave) {
        wave.offset += frameTime * .2f;
        trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
    });

    _orbitQuery.select(_world, [&](EntityId, components::Transform& trans) {
        trans.position = glm::rotateY(trans.position, frameTime);
    });

    _spinQuery.select(_world, [&](EntityId, components::Transform& trans, components::Spin const& spin) {
        trans.rotation = glm::angleAxis(spin.radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
    });

    _dingQuery.select(_world, [&, this](EntityId, components::Ding& ding) {
        ding.time += frameTime;
        if (ding.time > ding.period) {
            ding.time -= ding.period;
            _audioEngine.play(ding.sound.asset());
        }
    });
}

void up::Scene::flush() {
    _transformQuery.select(_world, [&](EntityId, components::Transform& trans) {
        trans.transform = glm::translate(trans.position) * glm::mat4_cast(trans.rotation);
    });
}

void up::Scene::render(RenderContext& ctx) {
    _renderableMeshQuery.select(_world, [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
        if (mesh.mesh.ready() && mesh.material.ready()) {
            mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.transform);
        }
    });
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
