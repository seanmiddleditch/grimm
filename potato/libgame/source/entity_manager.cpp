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
        EntityId const id{_nextEntityId++};
        _entities.insert(id);
        return id;
    }

    bool EntityManager::destroyEntity(EntityId entityId) noexcept {
        if (!_entities.erase(entityId)) {
            return false;
        }

        for (box<ComponentStorage>& comp : _components) {
            comp->remove(entityId);
        }
        return true;
    }

    bool EntityManager::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
        ComponentStorage* const component = _getComponent(componentId);
        if (component != nullptr) {
            return component->remove(entityId);
        }
        return false;
    }

    ComponentStorage& EntityManager::_registerComponent(box<ComponentStorage> storage) {
        UP_ASSERT(storage != nullptr);

        ComponentId const componentId = storage->componentId();
        UP_ASSERT(!_componentMap.contains(componentId));

        ComponentStorage* const result = storage.get();
        _components.push_back(std::move(storage));
        _componentMap.insert(componentId, result);
        return *result;
    }

    void* EntityManager::_addComponentRaw(EntityId entityId, ComponentId componentId) {
        UP_ASSERT(_entities.contains(entityId));
        ComponentStorage* const component = _getComponent(componentId);
        if (component != nullptr) {
            return component->add(entityId);
        }
        return nullptr;
    }

    ComponentStorage* EntityManager::_getComponent(ComponentId componentId) noexcept {
        auto const rs = _componentMap.find(componentId);
        return rs ? rs->value : nullptr;
    }

} // namespace up
