// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "transform_system.h"
#include "space.h"
#include "world.h"

#include <glm/gtx/rotate_vector.hpp>

void up::game::TransformSystem::start() {
    space().world().createQuery(_transformQuery);
}

void up::game::TransformSystem::update(float) {
    _transformQuery.select(space().world(), [&](EntityId, components::Transform& trans) {
        trans.transform = glm::translate(trans.position) * glm::mat4_cast(trans.rotation);
    });
}
