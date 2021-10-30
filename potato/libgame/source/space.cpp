// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "space.h"
#include "systems/demo_system.h"
#include "systems/render_system.h"
#include "systems/transform_system.h"

up::Space::Space(box<World> world) : _world(std::move(world)) {
    addSystem<game::TransformSystem>();
}

up::Space::~Space() = default;

void up::Space::start() {
    UP_ASSERT(_state == State::New);
    _state = State::Starting;

    for (auto& system : _systems) {
        system->start();
    }

    _state = State::Started;
}

void up::Space::stop() {
    UP_ASSERT(_state == State::Started);
    _state = State::Stopped;

    // stop in reverse order, to handle any hidden dependencies
    for (auto index = _systems.size(); index != 0; --index) {
        _systems[index - 1]->stop();
    }
}

void up::Space::update(float deltaTime) {
    UP_ASSERT(_state == State::Started);
    for (auto& system : _systems) {
        system->update(deltaTime);
    }
}

void up::Space::render(Renderer& renderer) {
    UP_ASSERT(_state == State::Started);
    for (auto& system : _systems) {
        system->render(renderer);
    }
}

void up::Space::addRenderSystem(Space& space, Renderer& renderer) {
    space.addSystem<game::RenderSystem>(renderer);
}

void up::Space::addDemoSystem(Space& space, class AudioEngine& audio) {
    space.addSystem<game::DemoSystem>(audio);
}
