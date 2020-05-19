// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include <potato/spud/span.h>

namespace up {
    /// Chunks are the storage mechanism of Entities and their Components. A Chunk
    /// is allocated to an Archetype and will store a list of Components according
    /// to the Archetype's specified layout.
    ///
    struct Chunk {
        static constexpr uint32 SizeBytes = 64 * 1024;

        struct alignas(64) Header {
            ArchetypeId archetype = ArchetypeId::Empty;
            unsigned int entities = 0;
            unsigned int capacity = 0;
            Chunk* next = nullptr;
        };

        using Payload = char[SizeBytes - sizeof(Header)];

        /// @brief Retrieves a span of EntityIds associated with this Chunk
        auto entities() noexcept -> span<EntityId> {
            return {reinterpret_cast<EntityId*>(payload), header.entities};
        }

        /// @brief Retrieves a span of EntityIds associated with this Chunk
        auto entities() const noexcept -> view<EntityId> {
            return {reinterpret_cast<EntityId const*>(payload), header.entities};
        }

        Header header;
        Payload payload;
    }; // namespace up

    static_assert(sizeof(Chunk) == Chunk::SizeBytes, "Chunk has incorrect size; possibly unexpected member padding");
} // namespace up
