// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/sequence.h"

#include <algorithm>

namespace up {
    EntityManager::EntityManager() = default;
    EntityManager::~EntityManager() = default;

    void* EntityManager::getComponentUnsafe(EntityId entityId, ComponentId componentId) noexcept {
        ComponentStorage* const component = _getComponent(componentId);
        if (component != nullptr) {
            return component->getUnsafe(entityId);
        }
        return nullptr;
    }

    auto EntityManager::createEntity() -> EntityId {
        EntityId const id{_entities.size()};
        _entities.push_back(id);
        return id;
    }

    bool EntityManager::deleteEntity(EntityId entityId) noexcept {
        for (auto& comp : _components) {
            comp->remove(entityId);
        }
        return erase(_entities, entityId) != 0;
    }

    bool EntityManager::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
        ComponentStorage* const component = _getComponent(componentId);
        if (component != nullptr) {
            return component->remove(entityId);
        }
        return false;
    }

    ComponentStorage& EntityManager::_registerComponent(box<ComponentStorage> storage) {
        _components.push_back(std::move(storage));
        return *_components.back();
    }

    void* EntityManager::_addComponentRaw(EntityId entityId, ComponentId componentId) {
        ComponentStorage* const component = _getComponent(componentId);
        if (component != nullptr) {
            return component->add(entityId);
        }
        return nullptr;
    }

    ComponentStorage* EntityManager::_getComponent(ComponentId componentId) noexcept {
        for (auto& comp : _components) {
            if (comp->componentId() == componentId) {
                return comp.get();
            }
        }
        return nullptr;
    }

} // namespace up
