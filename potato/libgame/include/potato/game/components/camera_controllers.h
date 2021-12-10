// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <glm/vec3.hpp>

namespace up {
    struct FlyCameraComponent {
        glm::vec3 relativeMovement = {0.f, 0.f, 0.f};
        glm::vec3 relativeMotion = {0.f, 0.f, 0.f};
        float moveMetersPerSec = 80.f;
        float rotateRadiansPerSec = 1.f;
    };
} // namespace up
