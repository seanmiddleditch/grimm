// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/render/context.h"
#include "potato/schema/components_schema.h"

namespace up {
    namespace {
        class RenderSystem final : public System {
        public:
            using System::System;

            void start() override { }
            void stop() override { }

            void update(float deltaTime) override;
            void render(RenderContext& ctx) override;
        };
    } // namespace

    void registerRenderSystem(Space& space) { space.addSystem<RenderSystem>(); }

    void RenderSystem::update(float) { }

    void RenderSystem::render(RenderContext& ctx) {
        space().entities().select<components::Mesh, components::Transform const>(
            [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
                if (mesh.mesh.ready() && mesh.material.ready()) {
                    mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.transform);
                }
            });
    }
} // namespace up
