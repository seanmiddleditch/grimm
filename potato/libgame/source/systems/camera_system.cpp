// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/camera_controllers.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/rotate_vector.hpp>

namespace up {
    namespace {
        class CameraSystem final : public System {
        public:
            using System::System;

            void update(float) override;
        };
    } // namespace

    void registerCameraSystem(Space& space) { space.addSystem<CameraSystem>(); }

    void CameraSystem::update(float frameTime) {
        space().entities().select<TransformComponent, FlyCameraComponent>(
            [frameTime](EntityId, TransformComponent& transform, FlyCameraComponent& flyCam) {
                // apply movement to transform
                glm::vec3 const move = transform.right() * flyCam.relativeMovement.x +
                    transform.up() * flyCam.relativeMovement.y + transform.forward() * flyCam.relativeMovement.z;
                transform.position += move * flyCam.moveMetersPerSec * frameTime;

                // apply mouse rotation to transform
                const glm::vec3 angles = glm::eulerAngles(transform.rotation);
                const float yaw =
                    glm::mod(angles.y - flyCam.relativeMotion.x * flyCam.rotateRadiansPerSec, glm::two_pi<float>());
                const float pitch = glm::clamp(
                    angles.x - flyCam.relativeMotion.y * flyCam.rotateRadiansPerSec,
                    -glm::half_pi<float>() * 0.9f,
                    glm::half_pi<float>() * 0.9f);
                transform.rotation = glm::quat({pitch, yaw, angles.z});

                // update movement speed from wheel
                flyCam.moveMetersPerSec = glm::clamp(flyCam.moveMetersPerSec + flyCam.relativeMotion.z, 0.1f, 20.f);

                // clear out applied motion/movement
                flyCam.relativeMotion = {0.f, 0.f, 0.f};
                flyCam.relativeMovement = {0.f, 0.f, 0.f};
            });
    }
} // namespace up
