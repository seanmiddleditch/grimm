// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/spud/int_types.h"

namespace up {
    static constexpr auto makeEntityId(uint64 mappingIndex, uint16 generation) noexcept -> EntityId {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 48) | mappingIndex);
    }

    static constexpr auto getEntityMappingIndex(EntityId entity) noexcept -> uint64 {
        return static_cast<uint64>(entity) & ((1ull << 48) - 1);
    }

    static constexpr auto getEntityGeneration(EntityId entity) noexcept -> uint16 {
        return static_cast<uint64>(entity) >> 48;
    }

    static constexpr auto makeMapped(uint16 generation, uint16 archetypeIndex, uint16 chunkIndex, uint16 entityIndex)
        -> uint64 {
        return (static_cast<uint64>(generation) << 48) | (static_cast<uint64>(archetypeIndex) << 32) |
            (static_cast<uint64>(chunkIndex) << 16) | static_cast<uint64>(entityIndex);
    }

    static constexpr auto getMappedChunk(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 16);
    }

    static constexpr auto getMappedIndex(uint64 mapping) noexcept -> uint16 { return static_cast<uint16>(mapping); }

    static constexpr auto getMappedGeneration(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 48);
    }

    static constexpr auto getMappedArchetype(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 32);
    }

    static constexpr auto makeFreeEntry(uint16 generation, uint64 index) noexcept -> uint64 {
        return (static_cast<uint64>(generation) << 48) | index;
    }
} // namespace up
