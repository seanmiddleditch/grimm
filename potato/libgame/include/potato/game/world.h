// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "chunk.h"
#include "shared_context.h"

#include "potato/spud/bit_set.h"
#include "potato/spud/box.h"
#include "potato/spud/concepts.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/rc.h"
#include "potato/spud/traits.h"
#include "potato/spud/vector.h"

namespace up {
    struct EcsSharedContext;
    template <typename...>
    class Query;

    /// A world contains a collection of Entities, Archetypes, and their associated Components.
    ///
    /// Entities from different Worlds cannot interact.
    ///
    class World {
    public:
        UP_GAME_API explicit World(rc<EcsSharedContext> context);
        UP_GAME_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        template <typename... Components>
        auto createQuery(Query<Components...>& query) -> Query<Components...>& {
            return query = Query<Components...>(_context);
        }

        /// Retrieve the chunks belonging to a specific archetype.
        ///
        /// @returns nullptr if the ArchetypeId is invalid
        ///
        UP_GAME_API auto chunksOf(ArchetypeId archetype) const noexcept -> view<Chunk*>;

        /// @brief View of all chunks allocated in the world.
        /// @return all chunks in the world.
        auto chunks() const noexcept -> view<Chunk*> { return _chunks; }

        /// Creates a new Entity with the provided list of Component data
        ///
        template <typename... Components>
        EntityId createEntity(identity_t<Components>&&... components) noexcept;

        /// Deletes an existing Entity
        ///
        UP_GAME_API void deleteEntity(EntityId entity) noexcept;

        /// Adds a new Component to an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        template <typename Component>
        void addComponent(EntityId entityId, identity_t<Component>&& component) noexcept;

        /// @brief Add a default-constructed component to an existing entity.
        /// @param entity The entity to add the componet to.
        /// @param typeInfo Metadata for the to-be-added component.
        /// @returns unsafe raw pointer to the created component.
        UP_GAME_API void* addComponentDefault(EntityId entity, ComponentId componentId);

        /// Removes a Component from an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        UP_GAME_API void removeComponent(EntityId entityId, ComponentId componentId) noexcept;

        /// @brief Removes a component from an entity.
        /// @tparam Component Component type to remove.
        /// @param entityId Entity to modify.
        template <typename Component>
        void removeComponent(EntityId entityId) noexcept {
            return removeComponent(entityId, makeComponentId<Component>());
        }

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is typically a slow operation. It will incur several table lookups
        /// and searches. This should only be used by tools and debug aids, typically,
        /// and a Query should be used for runtime code.
        ///
        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept;

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is a type-unsafe variant of getComponentSlow.
        ///
        UP_GAME_API void* getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept;

    private:
        struct AllocatedLocation {
            Chunk& chunk;
            uint16 chunkIndex;
            uint16 index;
        };

        struct ArchetypeChunkRange {
            uint32 offset = 0;
            uint32 length = 0;
        };

        struct EntityLocation {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
            uint16 chunk = 0;
            uint16 index = 0;
        };

        static constexpr uint64 freeEntityIndex = ~0ULL;

        UP_GAME_API EntityId _createEntityRaw(view<ComponentId> components, view<void*> data);
        UP_GAME_API void _addComponentRaw(EntityId entityId, ComponentId component, void* componentData) noexcept;
        void _deleteEntityData(ArchetypeId archetypeId, uint16 chunkIndex, uint16 index) noexcept;

        auto _allocateEntitySpace(ArchetypeId archetype) -> AllocatedLocation;
        auto _allocateEntityId(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId;
        void _recycleEntityId(EntityId entity) noexcept;

        UP_GAME_API auto _parseEntityId(EntityId entity) const noexcept -> EntityLocation;
        void _remapEntityId(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept;

        void _moveTo(
            ArchetypeId destArch,
            Chunk& destChunk,
            int destIndex,
            ArchetypeId srcArch,
            Chunk& srcChunk,
            int srcIndex);
        void _moveTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex);
        void _copyTo(
            ArchetypeId destArch,
            Chunk& destChunk,
            int destIndex,
            ComponentId srcComponent,
            void* srcData);
        void* _constructAt(ArchetypeId arch, Chunk& chunk, int index, ComponentId component);
        void _destroyAt(ArchetypeId arch, Chunk& chunk, int index);

        auto _addChunk(ArchetypeId archetype, Chunk* chunk) -> uint16;
        void _removeChunk(ArchetypeId archetype, int chunkIndex) noexcept;
        UP_GAME_API auto _getChunk(ArchetypeId archetype, int chunkIndex) const noexcept -> Chunk*;

        vector<ArchetypeChunkRange> _archetypeChunkRanges;
        vector<Chunk*> _chunks;
        vector<uint64> _entityMapping;
        uint64 _freeEntityHead = freeEntityIndex;
        rc<EcsSharedContext> _context;
    };

    template <typename... Components>
    EntityId World::createEntity(identity_t<Components>&&... components) noexcept {
        if constexpr (sizeof...(Components) != 0) {
            ComponentId const componentIds[] = {makeComponentId<Components>()...};
            void* const componentData[] = {&components...};

            return _createEntityRaw(componentIds, componentData);
        }
        else {
            return _createEntityRaw({}, {});
        }
    }

    template <typename Component>
    Component* World::getComponentSlow(EntityId entity) noexcept {
        return static_cast<Component*>(getComponentSlowUnsafe(entity, makeComponentId<Component>()));
    }

    template <typename Component>
    void World::addComponent(EntityId entityId, identity_t<Component>&& component) noexcept {
        _addComponentRaw(entityId, makeComponentId<Component>(), &component);
    }
} // namespace up
