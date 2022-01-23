// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/arcball.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

void up::ArcBall::handleInput(glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept {
    float const moveSpeed = glm::sqrt(boomLength);
    glm::quat const rot = glm::quat({pitch, yaw, 0});
    glm::vec3 const up = glm::rotate(rot, glm::vec3{0, 1, 0});
    glm::vec3 const right = glm::rotate(rot, glm::vec3{1, 0, 0});
    auto const move = relativeMovement.y * up * moveSpeed + -relativeMovement.x * right * moveSpeed;
    target += move * frameTime;

    yaw = glm::mod(yaw + relativeMotion.x, glm::two_pi<float>());
    pitch = glm::clamp(
        pitch - relativeMotion.y,
        -glm::half_pi<float>() + glm::epsilon<float>(),
        glm::half_pi<float>() - glm::epsilon<float>());
    boomLength = glm::max(1.f, boomLength - relativeMotion.z);
}

void up::ArcBall::applyToTransform(Transform& transform) const noexcept {
    glm::vec3 boom{0, 0, boomLength};
    boom = glm::rotate(boom, pitch, {1, 0, 0});
    boom = glm::rotate(boom, yaw, {0, 1, 0});

    transform.position = target + boom;
    transform.lookAt(target, {0, 1, 0});
}
