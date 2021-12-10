// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <glm/vec3.hpp>

namespace up::component {
    struct FlyCamera {
        glm::vec3 relativeMovement = {0.f, 0.f, 0.f};
        glm::vec3 relativeMotion = {0.f, 0.f, 0.f};
        float moveMetersPerSec = 10;
        float rotateRadiansPerSec = 1;
    };
} // namespace up::component
