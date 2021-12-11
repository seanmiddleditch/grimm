// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/shell/arcball.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

void up::ArcBallCameraController::handleInput(
    glm::vec3 relativeMovement,
    glm::vec3 relativeMotion,
    float frameTime) noexcept {
    float const moveSpeed = glm::sqrt(_boomLength);
    glm::quat const rot = glm::quat({_pitch, _yaw, 0});
    glm::vec3 const up = glm::rotate(rot, glm::vec3{0, 1, 0});
    glm::vec3 const right = glm::rotate(rot, glm::vec3{1, 0, 0});
    auto const move = relativeMovement.y * up * moveSpeed + -relativeMovement.x * right * moveSpeed;
    _target += move * frameTime;

    _yaw = glm::mod(_yaw + relativeMotion.x, glm::two_pi<float>());
    _pitch = glm::clamp(
        _pitch - relativeMotion.y,
        -glm::half_pi<float>() + glm::epsilon<float>(),
        glm::half_pi<float>() - glm::epsilon<float>());
    _boomLength = glm::max(1.f, _boomLength - relativeMotion.z);
}

void up::ArcBallCameraController::applyTransform(Transform& transform) noexcept {
    glm::vec3 boom{0, 0, _boomLength};
    boom = glm::rotate(boom, _pitch, {1, 0, 0});
    boom = glm::rotate(boom, _yaw, {0, 1, 0});

    transform.position = _target + boom;
    transform.lookAt(_target, {0, 1, 0});
}
