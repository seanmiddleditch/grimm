// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/rigidbody_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/rotate_vector.hpp>

namespace up {
    namespace {
        class PhysicsSystem final : public System {
        public:
            using System::System;

            void update(float) override;

        private:
            glm::vec3 _gravity = {0, -9, 0};
        };
    } // namespace

    void registerPhysicsSystem(Space& space) { space.addSystem<PhysicsSystem>(); }

    void PhysicsSystem::update(float deltaTime) {
        using namespace component;

        space().entities().select<Transform, RigidBody>([this, deltaTime](EntityId, Transform& trans, RigidBody& body) {
            if (trans.position.y > 0) {
                body.linearVelocity += _gravity * deltaTime;
                trans.position += body.linearVelocity * deltaTime;

                if (trans.position.y <= 0.f) {
                    trans.position.y = 0.f;
                    body.linearVelocity.y = 0.f;
                }
            }
        });
    }
} // namespace up
