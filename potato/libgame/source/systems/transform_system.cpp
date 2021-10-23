// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/query.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/game/world.h"
#include "potato/schema/components_schema.h"

#include <glm/gtx/rotate_vector.hpp>

namespace {
    using namespace up;

    class TransformSystem final : public System {
    public:
        using System::System;

        void start() override;
        void stop() override { }

        void update(float) override;
        void render(RenderContext&) override { }

    private:
        Query<components::Transform> _transformQuery;
    };
} // namespace

namespace up {
    void registerTransformSystem(Space& space) { space.addSystem<TransformSystem>(); }
} // namespace up

void TransformSystem::start() {
    space().world().createQuery(_transformQuery);
}

void TransformSystem::update(float) {
    _transformQuery.select(space().world(), [&](EntityId, components::Transform& trans) {
        trans.transform = glm::translate(trans.position) * glm::mat4_cast(trans.rotation);
    });
}
