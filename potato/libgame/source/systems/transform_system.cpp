// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/rotate_vector.hpp>

namespace up {
    namespace {
        class TransformSystem final : public System {
        public:
            using System::System;

            void update(float) override;
        };
    } // namespace

    void registerTransformSystem(Space& space) { space.addSystem<TransformSystem>(); }

    void TransformSystem::update(float) {
        space().entities().select<TransformComponent>([&](EntityId, TransformComponent& trans) {
            trans.transform = glm::translate(trans.position) * glm::mat4_cast(trans.rotation);
        });
    }
} // namespace up
