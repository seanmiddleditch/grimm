// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/schema/components_schema.h"

#include <glm/gtx/rotate_vector.hpp>

namespace {
    using namespace up;

    class PhysicsSystem final : public System {
    public:
        using System::System;

        void start() override { }
        void stop() override { }

        void update(float) override;
        void render(RenderContext&) override { }

    private:
        glm::vec3 _gravity = {0, -9, 0};
    };
} // namespace

namespace up {
    void registerPhysicsSystem(Space& space) { space.addSystem<PhysicsSystem>(); }
} // namespace up

void PhysicsSystem::update(float deltaTime) {
    space().entities().select<components::Transform, components::Body>(
        [this, deltaTime](EntityId, components::Transform& trans, components::Body& body) {
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
