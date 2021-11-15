// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"

#include <glm/vec3.hpp>

namespace up::component {
    struct RigidBody {
        glm::vec3 linearVelocity{0.f, 0.f, 0.f};
    };
} // namespace up::component
