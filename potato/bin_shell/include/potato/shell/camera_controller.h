// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/transform.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#pragma once

namespace up {
    class Camera;

    class CameraController {
    public:
        virtual ~CameraController() = default;
        virtual void apply(
            Transform& transform,
            glm::vec3 relativeMovement,
            glm::vec3 relativeMotion,
            float frameTime) noexcept = 0;
    };

    class ArcBallCameraController : public CameraController {
    public:
        explicit ArcBallCameraController(Transform const& transform) noexcept;
        void apply(Transform& transform, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept
            override;

    private:
        glm::vec3 _target = {0, 5, 0};
        float _boomLength = 10;
        float _yaw = 0;
        float _pitch = -glm::quarter_pi<float>();
    };
} // namespace up
