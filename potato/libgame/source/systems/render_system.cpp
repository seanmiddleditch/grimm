// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/query.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/game/world.h"
#include "potato/render/context.h"
#include "potato/schema/components_schema.h"

namespace {
    using namespace up;

    class RenderSystem final : public System {
    public:
        using System::System;

        void start() override;
        void stop() override;

        void update(float deltaTime) override;
        void render(RenderContext& ctx) override;

    private:
        Query<components::Mesh, components::Transform> _meshQuery;
    };
} // namespace

namespace up {
    void registerRenderSystem(Space& space) { space.addSystem<RenderSystem>(); }
} // namespace up

void RenderSystem::start() {
    space().world().createQuery(_meshQuery);
}

void RenderSystem::stop() { }

void RenderSystem::update(float) { }

void RenderSystem::render(RenderContext& ctx) {
    _meshQuery.select(space().world(), [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
        if (mesh.mesh.ready() && mesh.material.ready()) {
            mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.transform);
        }
    });
}
