// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "space.h"

up::Space::Space() = default;
up::Space::~Space() = default;

void up::Space::update(float deltaTime) {
    for (auto& system : _systems) {
        system->update(deltaTime);
    }
}

void up::Space::render(RenderContext& ctx) { }

void up::Space::addSystem(box<System> system) {
    _systems.push_back(std::move(system));
    _systems.back()->start();
}
