// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/query.h"
#include "potato/game/space.h"
#include "potato/game/system.h"
#include "potato/game/world.h"
#include "potato/render/context.h"
#include "potato/render/renderer.h"
#include "potato/schema/components_schema.h"

namespace {
    using namespace up;

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

    class RenderSystem final : public System {
    public:
        RenderSystem(Space& space, Renderer& renderer) : System(space), _renderer(renderer) { }

        void start() override;
        void stop() override;

        void update(float deltaTime) override;
        void render(Renderer&) override;

    private:
        Query<components::Mesh, components::Transform> _meshQuery;
        box<MeshRenderer> _meshRenderer;
        Renderer& _renderer;
    };
} // namespace

namespace up {
    void registerRenderSystem(Space& space, Renderer& renderer) { space.addSystem<RenderSystem>(renderer); }
} // namespace up

void RenderSystem::start() {
    space().world().createQuery(_meshQuery);

    if (!_meshRenderer) {
        _meshRenderer = new_box<MeshRenderer>(&_meshQuery, &space().world());
    }
    _renderer.createRendarable(_meshRenderer.get());
}

void RenderSystem::stop() { }

void RenderSystem::update(float) { }

void RenderSystem::render(Renderer&) { }

void MeshRenderer::onSchedule(up::RenderContext& ctx) { }

void MeshRenderer::onRender(up::RenderContext& ctx) {
    _meshQuery->select(*_world, [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
        if (mesh.mesh.ready() && mesh.material.ready()) {
            mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.transform);
        }
    });
}
