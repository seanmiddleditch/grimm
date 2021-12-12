// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/transform.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#pragma once

namespace up {
    class ArcBallCameraController {
    public:
        void UP_VECTORCALL handleInput(glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept;

        void UP_VECTORCALL setTarget(glm::vec3 target) noexcept { _target = target; }
        void setBoom(float boomLength) noexcept { _boomLength = glm::max(1.f, boomLength); }
        void setYawPitch(float yaw, float pitch) noexcept {
            _yaw = yaw;
            _pitch = pitch;
        }

        void applyTransform(Transform& transform) noexcept;

    private:
        glm::vec3 _target = {0, 5, 0};
        float _boomLength = 10;
        float _yaw = 0;
        float _pitch = -glm::quarter_pi<float>();
    };
} // namespace up
