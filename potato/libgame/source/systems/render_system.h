// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "components_schema.h"
#include "query.h"
#include "system.h"

#include "potato/render/renderer.h"

namespace up::game {
    class RenderSystem final : public System {
    public:
        explicit RenderSystem(Space& space, Renderer& renderer) : System(space), _renderer(renderer) { }

        void start() override;
        void stop() override;

        void update(float deltaTime) override;
        void render(Renderer& ctx) override;

    private:
        class MeshRenderer : public up::IRenderable {
        public:
            MeshRenderer(Query<components::Mesh, components::Transform>* meshQuery, World* world)
                : _meshQuery(meshQuery)
                , _world(world) { }

            void onSchedule(up::RenderContext& ctx) override;
            void onRender(up::RenderContext& ctx) override;

        private:
            Query<components::Mesh, components::Transform>* _meshQuery;
            World* _world;
        };

        Query<components::Mesh, components::Transform> _meshQuery;
        box<MeshRenderer> _meshRenderer;
        Renderer& _renderer;
    };
} // namespace up::game
