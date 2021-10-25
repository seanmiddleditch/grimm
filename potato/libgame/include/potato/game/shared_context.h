// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "chunk.h"
#include "component.h"
#include "layout.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/span.h"
#include "potato/spud/utility.h"
#include "potato/spud/vector.h"

namespace up {
    struct EcsSharedContext : shared<EcsSharedContext> {
        struct ComponentRecord {
            ComponentId id;
            ComponentTypeBase const* component = nullptr;
        };

        struct ArchetypeLayout {
            uint32 layoutOffset = 0;
            uint16 layoutLength = 0;
            uint16 maxEntitiesPerChunk = 0;
        };

        struct FindResult {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
        };

        UP_GAME_API auto findComponentTypeById(ComponentId id) const noexcept -> ComponentTypeBase const*;

        template <typename Component>
        auto findComponentByType() const noexcept -> ComponentTypeBase const* {
            return findComponentTypeById(makeComponentId<Component>());
        }

        auto acquireChunk() -> Chunk*;
        void recycleChunk(Chunk* chunk) noexcept;

        inline auto layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow>;

        auto acquireArchetype(ArchetypeId original, view<ComponentId> include, view<ComponentId> exclude)
            -> ArchetypeId;

        UP_GAME_API auto _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets)
            const noexcept -> bool;

        vector<ComponentRecord> components;
        vector<ArchetypeLayout> archetypes = {ArchetypeLayout{0, 0, sizeof(Chunk::Payload) / sizeof(EntityId)}};
        vector<LayoutRow> chunkRows;
        vector<box<Chunk>> allocatedChunks;
        Chunk* freeChunkHead = nullptr;
    };

    auto EcsSharedContext::layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow> {
        auto const index = to_underlying(archetype);
        UP_ASSERT(index >= 0 && index < archetypes.size());
        auto const& arch = archetypes[index];
        return chunkRows.subspan(arch.layoutOffset, arch.layoutLength);
    }

} // namespace up
