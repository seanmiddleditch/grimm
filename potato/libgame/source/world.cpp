// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/world.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/sequence.h"

#include <algorithm>

up::World::World() = default;

up::World::~World() = default;

void* up::World::getComponentSlowUnsafe(EntityId entityId, ComponentId componentId) noexcept {
    for (auto& comp : _components) {
        if (comp->componentId() == componentId) {
            return comp->getUnsafe(entityId);
        }
    }
    return nullptr;
}

auto up::World::createEntity() -> EntityId {
    EntityId const id { _entities.size() };
    _entities.push_back(id);
    return id;
}

bool up::World::deleteEntity(EntityId entityId) noexcept {
    for (auto& comp : _components) {
        comp->remove(entityId);
    }
    return erase(_entities, entityId) != 0;
}

bool up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    for (auto& comp : _components) {
        if (comp->componentId() == componentId) {
            return comp->remove(entityId);
        }
    }
    return false;
}

auto up::World::getComponentStorage(ComponentId componentId) noexcept -> ComponentStorage* {
    for (auto& comp : _components) {
        if (comp->componentId() == componentId) {
            return comp.get();
        }
    }
    return nullptr;
}

void up::World::_registerComponent(box<ComponentStorage> storage) {
    _components.push_back(std::move(storage));
}

void* up::World::_addComponentRaw(EntityId entityId, ComponentId componentId) {
    for (auto& comp : _components) {
        if (comp->componentId() == componentId) {
            return comp->add(entityId);
        }
    }
    return nullptr;
}
