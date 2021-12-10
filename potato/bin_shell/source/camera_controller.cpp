// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/shell/camera_controller.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

constexpr glm::vec3 forward{0, 0, -1};
constexpr glm::vec3 right{1, 0, 0};
constexpr glm::vec3 upward{0, 1, 0};

up::FlyCameraController::FlyCameraController(Transform const& transform) noexcept {
    auto view = -transform.forward();

    _pitch = asin(-view.y);
    _yaw = atan2(view.x, view.z);
}

void up::FlyCameraController::apply(
    Transform& transform,
    glm::vec3 relativeMovement,
    glm::vec3 relativeMotion,
    float frameTime) noexcept {
    glm::vec3 movement = transform.right() * relativeMovement.x * _speed +
        transform.up() * relativeMovement.y * _speed + transform.forward() * relativeMovement.z * _speed;

    _speed = glm::clamp(_speed + relativeMotion.z, 0.1f, 20.f);

    glm::vec3 pos = transform.position + movement * _moveSpeedPerSec * frameTime;

    _yaw = glm::mod(_yaw - relativeMotion.x * _rotateRadiansPerSec, glm::two_pi<float>());
    _pitch = glm::clamp(
        _pitch - relativeMotion.y * _rotateRadiansPerSec,
        -glm::half_pi<float>() * 0.9f,
        glm::half_pi<float>() * 0.9f);

    auto view = glm::rotate(forward, _pitch, right);
    view = glm::rotate(view, _yaw, upward);

    transform.position = pos;
    transform.lookAt(pos + view, upward);
}

up::ArcBallCameraController::ArcBallCameraController(Transform const& transform) noexcept {
    auto view = -transform.forward();

    _pitch = asin(-view.y);
    _yaw = atan2(view.x, view.z);
    _boomLength = distance(transform.position, _target);
}

void up::ArcBallCameraController::apply(
    Transform& transform,
    glm::vec3 relativeMovement,
    glm::vec3 relativeMotion,
    float frameTime) noexcept {
    auto const move = relativeMovement.y * transform.up() + -relativeMovement.x * transform.right();
    _target += move * frameTime;

    _yaw = glm::mod(_yaw + relativeMotion.x, glm::two_pi<float>());
    _pitch = glm::clamp(
        _pitch - relativeMotion.y,
        -glm::half_pi<float>() + glm::epsilon<float>(),
        glm::half_pi<float>() - glm::epsilon<float>());
    _boomLength = glm::clamp(_boomLength - relativeMotion.z, 1.f, 200.f);

    glm::vec3 pos{0, 0, _boomLength};
    pos = glm::rotate(pos, _pitch, right);
    pos = glm::rotate(pos, _yaw, upward);

    transform.position = pos;
    transform.lookAt(_target, upward);
}
