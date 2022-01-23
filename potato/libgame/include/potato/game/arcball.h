// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"
#include "potato/game/transform.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#pragma once

namespace up {
    struct ArcBall {
        glm::vec3 target = {0, 5, 0};
        float boomLength = 10;
        float yaw = 0;
        float pitch = -glm::quarter_pi<float>();

        UP_GAME_API void UP_VECTORCALL handleInput(glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept;
        UP_GAME_API void applyToTransform(Transform& transform) const noexcept;
    };
} // namespace up
