// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/entity_manager.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/sequence.h"

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
        UP_GUARD(component != nullptr, false);
        return component->remove(entityId);
    }

    void EntityManager::observe(RawComponentObserver& observer) {
        ComponentStorage* const component = _getComponent(observer.componentId());
        UP_GUARD_VOID(component != nullptr);
        component->observe(&observer);
    }

    void EntityManager::unobserve(RawComponentObserver& observer) {
        ComponentStorage* const component = _getComponent(observer.componentId());
        UP_GUARD_VOID(component != nullptr);
        component->unobserve(&observer);
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

    void* EntityManager::_addComponentRaw(EntityId entityId, ComponentId componentId, void const* source) {
        UP_GUARD(_entities.contains(entityId), nullptr);
        ComponentStorage* const component = _getComponent(componentId);
        UP_GUARD(component != nullptr, nullptr);
        return component->add(entityId, source);
    }

    ComponentStorage* EntityManager::_getComponent(ComponentId componentId) noexcept {
        auto const rs = _componentMap.find(componentId);
        return rs ? rs->value : nullptr;
    }

} // namespace up
