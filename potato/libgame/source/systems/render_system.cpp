// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "render_system.h"
#include "space.h"
#include "world.h"

#include "potato/render/context.h"

void up::game::RenderSystem::start() {
    space().world().createQuery(_meshQuery);
}

void up::game::RenderSystem::stop() { }

void up::game::RenderSystem::update(float) { }

void up::game::RenderSystem::render(RenderContext& ctx) {
    _meshQuery.select(space().world(), [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
        if (mesh.mesh.ready() && mesh.material.ready()) {
            mesh.mesh.asset()->render(ctx, mesh.material.asset(), trans.transform);
        }
    });
}
