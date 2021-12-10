// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/platform.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>

namespace up {
    struct Transform {
        glm::vec3 position;
        glm::quat rotation;
        float scale = 1.f;

        glm::vec3 UP_VECTORCALL forward() const noexcept { return rotation * glm::vec3{0.f, 0.f, -1.f}; }
        glm::vec3 UP_VECTORCALL right() const noexcept { return rotation * glm::vec3{1.f, 0.f, 0.f}; }
        glm::vec3 UP_VECTORCALL up() const noexcept { return rotation * glm::vec3{0.f, 1.f, 0.f}; }

        void UP_VECTORCALL lookAt(glm::vec3 at, glm::vec3 up = glm::vec3{0.f, 1.f, 0.f}) noexcept {
            glm::vec3 const view = normalize(at - position);
            glm::vec3 const right = normalize(cross(view, up));
            glm::vec3 const orthoUp = cross(right, view);
            rotation = glm::conjugate(glm::toQuat(lookAtRH(position, at, orthoUp)));
        }
    };
} // namespace up
