// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "world.h"
#include "entity_id.h"
#include "shared_context.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/find.h"
#include "potato/spud/sequence.h"

#include <algorithm>

namespace up {
    static auto findRowDesc(view<LayoutRow> layout, ComponentId component) noexcept -> LayoutRow const* {
        for (LayoutRow const& row : layout) {
            if (row.component == component) {
                return &row;
            }
        }
        return nullptr;
    }
} // namespace up

up::World::World(rc<EcsSharedContext> context) : _context(std::move(context)) {}

up::World::~World() = default;

auto up::World::chunksOf(ArchetypeId arch) const noexcept -> view<Chunk*> {
    auto const archIndex = to_underlying(arch);
    if (archIndex < 0 || archIndex >= _archetypeChunkRanges.size()) {
        return {};
    }
    auto const& range = _archetypeChunkRanges[archIndex];
    return _chunks.subspan(range.offset, range.length);
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _parseEntityId(entity); success) {
        auto const layout = _context->layoutOf(archetypeId);

        if (auto const row = findRowDesc(layout, component); row != nullptr) {
            auto& chunk = *_getChunk(archetypeId, chunkIndex);
            return chunk.payload + row->offset + row->width * index;
        }
    }
    return nullptr;
}

void up::World::deleteEntity(EntityId entity) noexcept {
    auto [success, archetypeId, chunkIndex, index] = _parseEntityId(entity);
    if (success) {
        _deleteEntityData(archetypeId, chunkIndex, index);
        _recycleEntityId(entity);
    }
}

void up::World::_deleteEntityData(ArchetypeId archetypeId, uint16 chunkIndex, uint16 index) noexcept {
    Chunk* chunk = _getChunk(archetypeId, chunkIndex);

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    //
    auto const lastIndex = static_cast<int>(chunk->header.entities - 1);
    if (static_cast<int>(index) != lastIndex) {
        auto const movedEntity = chunk->entities()[index] = chunk->entities()[lastIndex];
        _moveTo(archetypeId, *chunk, index, *chunk, lastIndex);
        _remapEntityId(movedEntity, archetypeId, chunkIndex, index);
    }

    _destroyAt(archetypeId, *chunk, lastIndex);

    // if this was the last entity, deallocate the whole chunk
    //
    if (--chunk->header.entities == 0) {
        _removeChunk(archetypeId, chunkIndex);
        _context->recycleChunk(chunk);
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    reflex::TypeInfo const* const typeInfo = _context->findComponentById(componentId);
    UP_ASSERT(typeInfo != nullptr);

    if (auto [success, archetypeId, chunkIndex, index] = _parseEntityId(entityId); success) {
        ArchetypeId newArchetype = _context->acquireArchetype(archetypeId, {}, {&typeInfo, 1});
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntitySpace(newArchetype);

        auto* oldChunk = _getChunk(archetypeId, chunkIndex);
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *oldChunk, index);

        newChunk.entities()[newIndex] = entityId;

        _deleteEntityData(archetypeId, chunkIndex, index);
        _remapEntityId(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

void* up::World::addComponentDefault(EntityId entityId, reflex::TypeInfo const& typeInfo) {
    if (auto [success, archetypeId, chunkIndex, index] = _parseEntityId(entityId); success) {
        // find the target archetype and allocate an entry in it
        reflex::TypeInfo const* infoPtr = &typeInfo;
        ArchetypeId newArchetype = _context->acquireArchetype(archetypeId, {&infoPtr, 1}, {});
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntitySpace(newArchetype);

        auto* chunk = _getChunk(archetypeId, chunkIndex);
        newChunk.entities()[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        void* const data = _constructAt(newArchetype, newChunk, newIndex, static_cast<ComponentId>(typeInfo.hash));

        _deleteEntityData(archetypeId, chunkIndex, index);
        _remapEntityId(entityId, newArchetype, newChunkIndex, newIndex);

        return data;
    }
    return nullptr;
}

void up::World::_addComponentRaw(
    EntityId entityId,
    reflex::TypeInfo const& typeInfo,
    void const* componentData) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _parseEntityId(entityId); success) {
        // find the target archetype and allocate an entry in it
        reflex::TypeInfo const* infoPtr = &typeInfo;
        ArchetypeId newArchetype = _context->acquireArchetype(archetypeId, {&infoPtr, 1}, {});
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntitySpace(newArchetype);

        auto* chunk = _getChunk(archetypeId, chunkIndex);
        newChunk.entities()[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        _copyTo(newArchetype, newChunk, newIndex, static_cast<ComponentId>(typeInfo.hash), componentData);

        _deleteEntityData(archetypeId, chunkIndex, index);
        _remapEntityId(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

auto up::World::_createEntityRaw(view<reflex::TypeInfo const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    ArchetypeId newArchetype = _context->acquireArchetype(ArchetypeId::Empty, components, {});
    auto [newChunk, newChunkIndex, newIndex] = _allocateEntitySpace(newArchetype);

    // Allocate EntityId
    auto const entity = _allocateEntityId(newArchetype, newChunkIndex, newIndex);
    static_cast<EntityId*>(static_cast<void*>(newChunk.payload))[newIndex] = entity;

    for (auto index : sequence(components.size())) {
        _copyTo(newArchetype, newChunk, newIndex, static_cast<ComponentId>(components[index]->hash), data[index]);
    }

    return entity;
}

auto up::World::_allocateEntitySpace(ArchetypeId archetype) -> AllocatedLocation {
    auto const chunks = chunksOf(archetype);

    for (auto chunkIndex : sequence(chunks.size())) {
        Chunk* const chunk = chunks[chunkIndex];
        if (chunk->header.entities < chunk->header.capacity) {
            uint16 index = chunk->header.entities++;
            return {*chunk, static_cast<uint16>(chunkIndex), index};
        }
    }

    Chunk* const chunk = _context->acquireChunk();
    auto const chunkIndex = _addChunk(archetype, chunk);
    uint16 const index = chunk->header.entities++;
    return {*chunk, chunkIndex, index};
}

auto up::World::_allocateEntityId(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId {
    // if there's a free ID, recycle it
    if (_freeEntityHead != freeEntityIndex) {
        auto const mappingIndex = _freeEntityHead;
        auto const mapping = _entityMapping[mappingIndex];
        auto const generation = getMappedGeneration(mapping);

        auto const newGeneration = generation + 1;

        _freeEntityHead = getMappedIndex(_entityMapping[mappingIndex]);

        _entityMapping[mappingIndex] = makeMapped(newGeneration, to_underlying(archetype), chunk, index);

        return makeEntityId(mappingIndex, newGeneration);
    }

    // there was no ID to recycle, so create a new one
    auto const mappingIndex = static_cast<uint32>(_entityMapping.size());
    _entityMapping.push_back(makeMapped(1, to_underlying(archetype), chunk, index));
    return makeEntityId(mappingIndex, 1);
}

void up::World::_recycleEntityId(EntityId entity) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const newGeneration = getEntityGeneration(entity) + 1;

    _entityMapping[entityMappingIndex] = makeFreeEntry(newGeneration != 0 ? newGeneration : 1, _freeEntityHead);

    _freeEntityHead = entityMappingIndex;
}

auto up::World::_parseEntityId(EntityId entity) const noexcept -> EntityLocation {
    auto const mappingIndex = getEntityMappingIndex(entity);
    if (mappingIndex < 0 || mappingIndex >= _entityMapping.size()) {
        return {false};
    }

    auto const mapped = _entityMapping[mappingIndex];
    auto const mappedGen = getMappedGeneration(_entityMapping[mappingIndex]);
    auto const entityGen = getEntityGeneration(entity);
    if (mappedGen != entityGen) {
        return {false};
    }

    return {true, ArchetypeId(getMappedArchetype(mapped)), getMappedChunk(mapped), getMappedIndex(mapped)};
}

void up::World::_remapEntityId(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const mapped = _entityMapping[entityMappingIndex];
    auto const mappedGen = getMappedGeneration(mapped);
    UP_ASSERT(mappedGen == getEntityGeneration(entity));
    _entityMapping[entityMappingIndex] = makeMapped(mappedGen, to_underlying(newArchetype), newChunk, newIndex);
}

void up::World::_moveTo(
    ArchetypeId destArch,
    Chunk& destChunk,
    int destIndex,
    ArchetypeId srcArch,
    Chunk& srcChunk,
    int srcIndex) {
    auto const srcLayout = _context->layoutOf(srcArch);
    for (LayoutRow const& row : _context->layoutOf(destArch)) {
        if (auto const srcRow = findRowDesc(srcLayout, row.component); srcRow != nullptr) {
            row.typeInfo->ops.moveAssignment(
                destChunk.payload + row.offset + row.width * destIndex,
                srcChunk.payload + srcRow->offset + srcRow->width * srcIndex);
        }
    }
}

void up::World::_moveTo(ArchetypeId arch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex) {
    for (LayoutRow const& layout : _context->layoutOf(arch)) {
        layout.typeInfo->ops.moveAssignment(
            destChunk.payload + layout.offset + layout.width * destIndex,
            srcChunk.payload + layout.offset + layout.width * srcIndex);
    }
}

void up::World::_copyTo(
    ArchetypeId destArch,
    Chunk& destChunk,
    int destIndex,
    ComponentId srcComponent,
    void const* srcData) {
    auto const destRow = findRowDesc(_context->layoutOf(destArch), srcComponent);
    destRow->typeInfo->ops.copyConstructor(destChunk.payload + destRow->offset + destRow->width * destIndex, srcData);
}

void* up::World::_constructAt(ArchetypeId arch, Chunk& chunk, int index, ComponentId component) {
    auto const row = findRowDesc(_context->layoutOf(arch), component);
    void* data = chunk.payload + row->offset + row->width * index;
    row->typeInfo->ops.defaultConstructor(data);
    return data;
}

void up::World::_destroyAt(ArchetypeId arch, Chunk& chunk, int index) {
    for (LayoutRow const& layout : _context->layoutOf(arch)) {
        layout.typeInfo->ops.destructor(chunk.payload + layout.offset + layout.width * index);
    }
}

auto up::World::_addChunk(ArchetypeId arch, Chunk* chunk) -> uint16 {
    UP_ASSERT(chunk != nullptr);

    auto const archIndex = to_underlying(arch);
    UP_ASSERT(archIndex >= 0 && archIndex < _context->archetypes.size());

    chunk->header.archetype = arch;
    chunk->header.capacity = _context->archetypes[archIndex].maxEntitiesPerChunk;

    if (archIndex >= _archetypeChunkRanges.size()) {
        _archetypeChunkRanges.resize(archIndex + 1, {static_cast<uint32>(_chunks.size()), 0});
    }

    auto& range = _archetypeChunkRanges[archIndex];

    auto const chunkIndex = narrow_cast<uint16>(range.length);

    _chunks.insert(_chunks.begin() + range.offset + range.length, chunk);
    ++range.length;

    for (auto& updateRange : _archetypeChunkRanges.subspan(archIndex + 1)) {
        ++updateRange.offset;
    }

    return chunkIndex;
}

void up::World::_removeChunk(ArchetypeId arch, int chunkIndex) noexcept {
    auto const archIndex = to_underlying(arch);
    UP_ASSERT(archIndex >= 0 && archIndex < _archetypeChunkRanges.size());

    auto& range = _archetypeChunkRanges[archIndex];
    UP_ASSERT(chunkIndex >= 0 && chunkIndex < static_cast<int>(range.length));

    _chunks.erase(_chunks.begin() + range.offset + chunkIndex);
    --range.length;

    for (auto& updateRange : _archetypeChunkRanges.subspan(archIndex + 1)) {
        --updateRange.offset;
    }
}

auto up::World::_getChunk(ArchetypeId arch, int chunkIndex) const noexcept -> Chunk* {
    auto const archIndex = to_underlying(arch);
    if (archIndex < 0 || archIndex >= _archetypeChunkRanges.size()) {
        return nullptr;
    }
    auto& range = _archetypeChunkRanges[archIndex];
    UP_ASSERT(chunkIndex >= 0 && chunkIndex < static_cast<int>(range.length));
    return _chunks[range.offset + chunkIndex];
}
