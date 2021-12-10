// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/camera_component.h"
#include "potato/game/components/mesh_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/render/context.h"

namespace up {
    namespace {
        class RenderSystem final : public System {
        public:
            using System::System;

            void update(float deltaTime) override;
            void render(RenderContext& ctx) override;
        };
    } // namespace

    void registerRenderSystem(Space& space) { space.addSystem<RenderSystem>(); }

    void RenderSystem::update(float) { }

    void RenderSystem::render(RenderContext& ctx) {
        space().entities().select<CameraComponent, TransformComponent const>(
            [&](EntityId, CameraComponent& camera, TransformComponent const& trans) {
                ctx.applyCameraPerspective(trans.position, trans.forward(), trans.up());
            });

        space().entities().select<MeshComponent, TransformComponent const>(
            [&](EntityId, MeshComponent& mesh, TransformComponent const& trans) {
                if (mesh.mesh.ready() && mesh.material.ready()) {
                    mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.matrix);
                }
            });
    }
} // namespace up
