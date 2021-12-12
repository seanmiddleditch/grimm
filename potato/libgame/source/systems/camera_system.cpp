// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/camera_controllers.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/projection.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

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
                glm::vec3 moveScale = glm::vec3{1, 1, -1} * flyCam.moveMetersPerSec * frameTime;
                transform.position += glm::rotate(transform.rotation, flyCam.relativeMovement * moveScale);

                // apply horizontal mouse movement
                flyCam.yaw += std::fmod(-flyCam.relativeMotion.x * flyCam.rotateRadiansPerSec, glm::two_pi<float>());
                if (flyCam.yaw < 0.f) {
                    flyCam.yaw += glm::two_pi<float>();
                }

                // apply vertical mouse movement; clamp pitch to only look straight up or down (but not upside down)
                flyCam.pitch = glm::clamp(
                    flyCam.pitch - flyCam.relativeMotion.y * flyCam.rotateRadiansPerSec,
                    -glm::half_pi<float>(),
                    glm::half_pi<float>());
                transform.rotation = glm::quat({flyCam.pitch, flyCam.yaw, 0});

                // update movement speed from wheel
                flyCam.moveMetersPerSec = glm::clamp(flyCam.moveMetersPerSec + flyCam.relativeMotion.z, 0.1f, 200.f);

                // clear out applied motion/movement
                flyCam.relativeMotion = {0.f, 0.f, 0.f};
                flyCam.relativeMovement = {0.f, 0.f, 0.f};
            });
    }
} // namespace up
