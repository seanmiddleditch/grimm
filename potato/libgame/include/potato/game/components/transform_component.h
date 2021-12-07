// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace up::component {
    struct Transform {
        glm::vec3 position;
        glm::quat rotation;
        glm::mat4x4 transform;

        glm::vec3 forward() const noexcept { return rotation * glm::vec3(0, 0, -1); }
        glm::vec3 right() const noexcept { return rotation * glm::vec3(1, 0, 0); }
        glm::vec3 up() const noexcept { return rotation * glm::vec3(0, 1, 0); }
    };
} // namespace up::component
