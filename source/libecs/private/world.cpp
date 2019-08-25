// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/entity.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include <algorithm>

#include "location.h"

namespace up {
    static auto findLayout(Archetype const& archetype, ComponentId component) noexcept -> ChunkRowDesc const* {
        for (ChunkRowDesc const& row : archetype.chunkLayout) {
            if (row.component == component) {
                return &row;
            }
        }
        return nullptr;
    }

    static constexpr auto rowAt(char* data, uint32 offset, uint32 width, uint32 index) noexcept -> void* {
        return data + offset + width * index;
    }

    static constexpr auto rowAt(char* data, uint32 offset) noexcept -> void* {
        return data + offset;
    }
} // namespace up

up::World::World() = default;

up::World::~World() = default;

auto up::World::version() const noexcept -> uint32 {
    return _archetypes.version();
}

auto up::World::archetypes() const noexcept -> view<box<Archetype>> {
    return _archetypes.archetypes();
}

auto up::World::getArchetype(ArchetypeId arch) const noexcept -> Archetype const* {
    return _archetypes.getArchetype(arch);
}

auto up::World::selectArchetypes(view<ComponentId> componentIds, delegate_ref<SelectSignature> callback) const -> int {
    return _archetypes.selectArchetypes(componentIds, callback);
}

void up::World::deleteEntity(EntityId entity) noexcept {
    Location location;
    if (!_tryGetLocation(entity, location)) {
        return;
    }

    _deleteLocation(location);
    _entities.recycle(entity);
}

void up::World::_deleteLocation(Location const& location) noexcept {
    auto lastChunkIndex = location.archetype->chunks.size() - 1;
    Chunk& lastChunk = *location.archetype->chunks[lastChunkIndex];

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    if (location.chunkIndex != lastChunkIndex && location.subIndex != lastChunk.header.entities) {
        auto lastSubIndex = lastChunk.header.entities - 1;

        for (auto const& layout : location.archetype->chunkLayout) {
            ComponentMeta const& meta = *layout.meta;

            void* pointer = rowAt(location.chunk->data, layout.offset, layout.width, location.subIndex);
            void* lastPointer = rowAt(lastChunk.data, layout.offset, layout.width, lastSubIndex);

            // ensure EntityId index is updated
            if (layout.component == getComponentId<Entity>()) {
                _entities.setIndex(static_cast<Entity const*>(lastPointer)->id, location.chunkIndex, location.subIndex);
            }

            meta.relocate(pointer, lastPointer);
            meta.destroy(lastPointer);
        }
    }
    else {
        // We were already the last element, just clean up our memory
        for (auto const& layout : location.archetype->chunkLayout) {
            ComponentMeta const& meta = *layout.meta;

            void* lastPointer = rowAt(location.chunk->data, layout.offset, layout.width, location.subIndex);

            meta.destroy(lastPointer);
        }
    }

    if (--lastChunk.header.entities == 0) {
        _chunks.recycle(std::move(location.archetype->chunks[lastChunkIndex]));
        location.archetype->chunks.pop_back();
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    Location location;
    if (!_tryGetLocation(entityId, location)) {
        return;
    }

    // figure out which layout entry is being removed
    auto iter = find(location.archetype->chunkLayout, componentId, {}, &ChunkRowDesc::component);
    if (iter == location.archetype->chunkLayout.end()) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
    auto out = copy(location.archetype->chunkLayout.begin(), iter, newComponentMetas, &ChunkRowDesc::meta);
    copy(iter + 1, location.archetype->chunkLayout.end(), out, &ChunkRowDesc::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->chunkLayout.size() - 1);

    // find the target archetype and allocate an entry in it
    Archetype* newArchetype = _archetypes.createArchetype(newComponentMetaSpan);

    auto newEntityIndex = newArchetype->entityCount++;
    auto newChunkIndex = newEntityIndex / newArchetype->maxEntitiesPerChunk;
    auto newSubIndex = newEntityIndex % newArchetype->maxEntitiesPerChunk;

    if (newChunkIndex == newArchetype->chunks.size()) {
        newArchetype->chunks.push_back(_chunks.allocate());
    }

    Chunk& newChunk = *newArchetype->chunks[newChunkIndex];
    ++newChunk.header.entities;

    for (ChunkRowDesc const& layout : newArchetype->chunkLayout) {
        auto const originalLayout = findLayout(*location.archetype, layout.component);
        UP_ASSERT(originalLayout != nullptr);

        void* newRawPointer = rowAt(newChunk.data, layout.offset, layout.width, newSubIndex);
        void* originalRawPointer = rowAt(location.chunk->data, originalLayout->offset, originalLayout->offset, location.subIndex);

        layout.meta->relocate(newRawPointer, originalRawPointer);
    }

    // remove old entity
    _deleteLocation(location);

    // update mapping
    _entities.setArchetype(entityId, newArchetype->id, newChunkIndex, newSubIndex);
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept {
    Location location;
    if (!_tryGetLocation(entityId, location)) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
    newComponentMetas[0] = componentMeta;
    copy(location.archetype->chunkLayout.begin(), location.archetype->chunkLayout.end(), newComponentMetas + 1, &ChunkRowDesc::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->chunkLayout.size() + 1);

    // find the target archetype and allocate an entry in it
    Archetype* newArchetype = _archetypes.createArchetype(newComponentMetaSpan);

    auto newEntityIndex = newArchetype->entityCount++;
    auto newChunkIndex = newEntityIndex / newArchetype->maxEntitiesPerChunk;
    auto newSubIndex = newEntityIndex % newArchetype->maxEntitiesPerChunk;

    if (newChunkIndex == newArchetype->chunks.size()) {
        newArchetype->chunks.push_back(_chunks.allocate());
    }

    Chunk& newChunk = *newArchetype->chunks[newChunkIndex];
    ++newChunk.header.entities;

    for (ChunkRowDesc const& layout : newArchetype->chunkLayout) {
        void* newRawPointer = rowAt(newChunk.data, layout.offset, layout.width, newSubIndex);

        // either copy existing component or install the new one
        auto const originalLayout = findLayout(*location.archetype, layout.component);
        if (originalLayout != nullptr) {
            void* originalRawPointer = rowAt(location.chunk->data, originalLayout->offset, originalLayout->width, location.subIndex);

            layout.meta->relocate(newRawPointer, originalRawPointer);
        }
        else {
            layout.meta->copy(newRawPointer, componentData);
        }
    }

    // remove old entity
    _deleteLocation(location);

    // update mapping
    _entities.setArchetype(entityId, newArchetype->id, newChunkIndex, location.subIndex);
}

auto up::World::_createEntityRaw(view<ComponentMeta const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    auto& archetype = *_archetypes.createArchetype(components);

    UP_ASSERT(components.size() == archetype.chunkLayout.size() - 1);
    UP_ASSERT(components.size() == data.size());

    auto const entityIndex = archetype.entityCount++;

    // Determine destination location Chunk
    auto const chunkIndex = entityIndex / archetype.maxEntitiesPerChunk;
    auto const subIndex = entityIndex % archetype.maxEntitiesPerChunk;

    UP_ASSERT(chunkIndex <= archetype.chunks.size());

    if (chunkIndex == archetype.chunks.size()) {
        archetype.chunks.push_back(_chunks.allocate());
    }

    Chunk& chunk = *archetype.chunks[chunkIndex];
    ++chunk.header.entities;

    // Allocate EntityId
    auto const entity = _entities.allocate(archetype.id, chunkIndex, subIndex);
    auto const entityLayout = findLayout(archetype, getComponentId<Entity>());
    void* rawEntityPointer = rowAt(chunk.data, entityLayout->offset, entityLayout->width, subIndex);
    static_cast<Entity*>(rawEntityPointer)->id = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentMeta const& meta = *components[index];

        auto const layout = findLayout(archetype, meta.id);
        UP_ASSERT(layout != nullptr);
        UP_ASSERT(layout->meta == &meta);
        UP_ASSERT(layout->width == meta.size);

        void* rawPointer = rowAt(chunk.data, layout->offset, layout->width, subIndex);

        meta.copy(rawPointer, data[index]);
    }

    return entity;
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    Location location;
    if (!_tryGetLocation(entity, location)) {
        return nullptr;
    }

    auto const layout = findLayout(*location.archetype, component);
    if (layout == nullptr) {
        return nullptr;
    }

    return rowAt(location.chunk->data, layout->offset, layout->width, location.subIndex);
}

auto up::World::_tryGetLocation(EntityId entityId, Location& location) const noexcept -> bool {
    ArchetypeId archetype;
    if (!_entities.tryParse(entityId, archetype, location.chunkIndex, location.subIndex)) {
        return false;
    }

    location.archetype = _archetypes.getArchetype(archetype);
    UP_ASSERT(location.archetype != nullptr);

    UP_ASSERT(location.chunkIndex < location.archetype->chunks.size());
    location.chunk = location.archetype->chunks[location.chunkIndex];
    UP_ASSERT(location.subIndex < location.chunk->header.entities);

    return true;
}
