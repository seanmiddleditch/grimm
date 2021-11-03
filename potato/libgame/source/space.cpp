// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/space.h"

namespace up {
    extern void registerDemoSystem(Space& space, AudioEngine& audioEngine);
    extern void registerPhysicsSystem(Space& space);
    extern void registerRenderSystem(Space& space);
    extern void registerTransformSystem(Space& space);
    extern void registerComponents(Space& space);

    Space::Space() {
        registerComponents(*this);
        registerTransformSystem(*this);
        registerRenderSystem(*this);
    }

    Space::~Space() = default;

    void Space::start() {
        UP_ASSERT(_state == State::New);
        _state = State::Starting;

        for (auto& system : _systems) {
            system->start();
        }

        _state = State::Started;
    }

    void Space::stop() {
        UP_ASSERT(_state == State::Started);
        _state = State::Stopped;

        // stop in reverse order, to handle any hidden dependencies
        for (auto index = _systems.size(); index != 0; --index) {
            _systems[index - 1]->stop();
        }
    }

    void Space::update(float deltaTime) {
        UP_ASSERT(_state == State::Started);
        for (auto& system : _systems) {
            system->update(deltaTime);
        }
    }

    void Space::render(RenderContext& ctx) {
        UP_ASSERT(_state == State::Started);
        for (auto& system : _systems) {
            system->render(ctx);
        }
    }

    void Space::addDemoSystem(Space& space, class AudioEngine& audio) {
        registerDemoSystem(space, audio);
        registerPhysicsSystem(space);
    }
} // namespace up
