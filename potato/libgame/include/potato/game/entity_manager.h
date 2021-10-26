// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"
#include "component.h"

#include "potato/spud/bit_set.h"
#include "potato/spud/box.h"
#include "potato/spud/concepts.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/rc.h"
#include "potato/spud/traits.h"
#include "potato/spud/vector.h"

namespace up {
    class Query;

    /// Contains a collection of Entities and their associated Components.
    ///
    class EntityManager {
    public:
        UP_GAME_API EntityManager();
        UP_GAME_API ~EntityManager();

        EntityManager(EntityManager&&) = delete;
        EntityManager& operator=(EntityManager&&) = delete;

        /// Creates a new Entity
        ///
        UP_GAME_API EntityId createEntity();

        /// Creates a new Entity with given component
        ///
        template <typename... Components>
        EntityId createEntity(identity_t<Components>&&... components);

        /// Deletes an existing Entity
        ///
        UP_GAME_API bool deleteEntity(EntityId entity) noexcept;

        /// Adds a new Component to an existing Entity.
        ///
        template <typename Component>
        Component& addComponent(EntityId entityId, identity_t<Component>&& component) noexcept;

        /// Adds a new Component to an existing Entity.
        ///
        template <typename Component>
        Component& addComponent(EntityId entityId) noexcept;

        /// Adds a new Component to an existing Entity.
        ///
        UP_GAME_API void addComponent(EntityId entityId, ComponentId componentId) noexcept;

        /// Removes a Component from an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        UP_GAME_API bool removeComponent(EntityId entityId, ComponentId componentId) noexcept;

        /// @brief Removes a component from an entity.
        /// @tparam Component Component type to remove.
        /// @param entityId Entity to modify.
        template <typename Component>
        bool removeComponent(EntityId entityId) noexcept {
            return removeComponent(entityId, makeComponentId<Component>());
        }

        /// Invokes callback for every Entity matching the signature
        template <typename... Components, typename Callback>
            requires is_invocable_v<Callback, EntityId, Components&...>
        void select(Callback&& callback);

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is typically a slow operation. It will incur several table lookups
        /// and searches. This should only be used by tools and debug aids, typically,
        /// and a Query should be used for runtime code.
        ///
        template <typename Component>
        Component* getComponentSlow(EntityId entityId) noexcept;

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is a type-unsafe variant of getComponentSlow.
        ///
        UP_GAME_API void* getComponentSlowUnsafe(EntityId entityId, ComponentId componentId) noexcept;

        /// Registers a new component type
        template <typename Component>
        void registerComponent();

        /// Retrieves the component storage for a given component type
        UP_GAME_API ComponentStorage* getComponentStorage(ComponentId componentId) noexcept;

    private:
        UP_GAME_API void _registerComponent(box<ComponentStorage> storage);
        UP_GAME_API void* _addComponentRaw(EntityId entityId, ComponentId componentId);

        template <typename Callback, typename... Components, size_t... Indices>
        void _select(Callback&& callback, typelist<Components...>, std::index_sequence<Indices...>);

        vector<EntityId> _entities;
        vector<box<ComponentStorage>> _components;
    };

    template <typename... Components>
    EntityId EntityManager::createEntity(identity_t<Components>&&... components) {
        auto const entityId = createEntity();
        int const _[] = {(addComponent(entityId, std::move(components)), 0)...};
        return entityId;
    }

    template <typename Component>
    Component* EntityManager::getComponentSlow(EntityId entity) noexcept {
        return static_cast<Component*>(getComponentSlowUnsafe(entity, makeComponentId<Component>()));
    }

    template <typename Component>
    Component& EntityManager::addComponent(EntityId entityId, identity_t<Component>&& component) noexcept {
        void* const data = _addComponentRaw(entityId, makeComponentId<Component>());
        UP_ASSERT(data != nullptr);
        return *static_cast<Component*>(data) = std::move(component);
    }

    template <typename... Components, typename Callback>
        requires is_invocable_v<Callback, EntityId, Components&...>
    void EntityManager::select(Callback&& callback) {
        _select(callback, typelist<Components...>{}, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename Component>
    void EntityManager::registerComponent() {
        _registerComponent(new_box<TypedComponentStorage<Component>>());
    }

    template <typename Callback, typename... Components, size_t... Indices>
    void EntityManager::_select(Callback&& callback, typelist<Components...>, std::index_sequence<Indices...>) {
        ComponentId const componentIds[] = {makeComponentId<Components>()...};
        ComponentStorage* componentStorages[] = {getComponentStorage(componentIds[Indices])...};
        void* componentData[sizeof...(Components)] = {};

        for (size_t index = 0; index != sizeof...(Components); ++index) {
            UP_ASSERT(componentStorages[index] != nullptr);
            if (componentStorages[index] == nullptr) {
                return;
            }
        }

        componentStorages[0]->forEachUnsafe([&, this](EntityId entityId, void* data) {
            componentData[0] = data;
            for (size_t index = 1; index != sizeof...(Components); ++index) {
                componentData[index] = componentStorages[index]->getUnsafe(entityId);
                if (componentData[index] == nullptr) {
                    return true;
                }
            }

            callback(entityId, *(static_cast<Components*>(componentData[Indices]))...);

            return true;
        });
    }
} // namespace up
