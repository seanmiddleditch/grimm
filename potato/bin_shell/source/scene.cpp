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

up::Scene::Scene(Universe& universe)
    : Space(universe.createWorld())
    , _universe(universe) {}

up::Scene::~Scene() = default;

void up::Scene::update(float frameTime) {
    if (_playing) {

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
