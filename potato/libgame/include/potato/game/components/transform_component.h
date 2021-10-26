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
    };
} // namespace up::component
