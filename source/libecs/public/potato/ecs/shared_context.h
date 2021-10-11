// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "chunk.h"
#include "layout.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/span.h"
#include "potato/spud/utility.h"
#include "potato/spud/vector.h"

namespace up {
    struct EcsSharedContext : shared<EcsSharedContext> {
        struct ArchetypeLayout {
            uint32 layoutOffset = 0;
            uint16 layoutLength = 0;
            uint16 maxEntitiesPerChunk = 0;
        };

        struct FindResult {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
        };

        auto findComponentById(ComponentId id) const noexcept -> reflex::TypeInfo const*;
        auto findComponentByName(string_view name) const noexcept -> reflex::TypeInfo const*;

        template <typename Component>
        auto findComponentByType() const noexcept -> reflex::TypeInfo const*;

        auto acquireChunk() -> Chunk*;
        void recycleChunk(Chunk* chunk) noexcept;

        inline auto layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow>;

        auto acquireArchetype(
            ArchetypeId original,
            view<reflex::TypeInfo const*> include,
            view<reflex::TypeInfo const*> exclude) -> ArchetypeId;

        UP_ECS_API auto _findComponentByTypeHash(uint64 typeHash) const noexcept -> reflex::TypeInfo const*;
        UP_ECS_API auto _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets)
            const noexcept -> bool;

        vector<reflex::TypeInfo const*> components;
        vector<ArchetypeLayout> archetypes = {ArchetypeLayout{0, 0, sizeof(Chunk::Payload) / sizeof(EntityId)}};
        vector<LayoutRow> chunkRows;
        vector<box<Chunk>> allocatedChunks;
        Chunk* freeChunkHead = nullptr;
    };

    template <typename Component>
    auto EcsSharedContext::findComponentByType() const noexcept -> reflex::TypeInfo const* {
        static const uint64 hash = reflex::TypeHolder<Component>::get().hash;
        return _findComponentByTypeHash(hash);
    }

    auto EcsSharedContext::layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow> {
        auto const index = to_underlying(archetype);
        UP_ASSERT(index >= 0 && index < archetypes.size());
        auto const& arch = archetypes[index];
        return chunkRows.subspan(arch.layoutOffset, arch.layoutLength);
    }

} // namespace up
