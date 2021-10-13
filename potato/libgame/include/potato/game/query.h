// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "shared_context.h"
#include "world.h"

#include "potato/spud/span.h"
#include "potato/spud/traits.h"
#include "potato/spud/typelist.h"
#include "potato/spud/utility.h"
#include "potato/spud/vector.h"

namespace up {
    /// A Query is used to select a list of Archetypes that provide a particular set of Components,
    /// used to efficiency enumerate all matching Entities.
    template <typename... Components>
    class Query {
    public:
        static_assert(sizeof...(Components) != 0, "Empty Query objects are not allowed");

        explicit Query(rc<EcsSharedContext> context) : _context(std::move(context)) {}

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each Chunk belonging to the Archetypes, with appropriate pointers.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback>
        void selectChunks(
            World& world,
            Callback&& callback) requires is_invocable_v<Callback, size_t, EntityId const*, Components*...>;

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each entity.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback>
        void select(World& world, Callback&& callback) requires is_invocable_v<Callback, EntityId, Components&...>;

    private:
        struct Match {
            ArchetypeId archetype;
            int offsets[sizeof...(Components)];
        };

        void _match();
        template <typename Callback, size_t... Indices>
        void _executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>) const;
        template <typename Callback, size_t... Indices>
        void _execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const;

        vector<Match> _matches;
        size_t _matchIndex = 0;
        rc<EcsSharedContext> _context;
    };

    template <typename... Components>
    template <typename Callback>
    void Query<Components...>::selectChunks(
        World& world,
        Callback&& callback) requires is_invocable_v<Callback, size_t, EntityId const*, Components*...> {
        _match();
        _executeChunks(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <typename Callback>
    void Query<Components...>::select(
        World& world,
        Callback&& callback) requires is_invocable_v<Callback, EntityId, Components&...> {
        _match();
        _execute(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    void Query<Components...>::_match() {
        if (_matchIndex >= _context->archetypes.size()) {
            return;
        }

        ComponentId const components[sizeof...(Components)] = {
            static_cast<ComponentId>(_context->findComponentByType<Components>()->hash)...};

        for (; _matchIndex < _context->archetypes.size(); ++_matchIndex) {
            auto& match = _matches.push_back({ArchetypeId(_matchIndex)});
            if (!_context->_bindArchetypeOffets(match.archetype, components, match.offsets)) {
                _matches.pop_back();
            }
        }
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>)
        const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.chunksOf(match.archetype)) {
                callback(
                    chunk->header.entities,
                    static_cast<EntityId const*>(static_cast<void*>(chunk->payload)),
                    static_cast<Components*>(static_cast<void*>(chunk->payload + match.offsets[Indices]))...);
            }
        }
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.chunksOf(match.archetype)) {
                for (unsigned index = 0; index < chunk->header.entities; ++index) {
                    callback(
                        *(static_cast<EntityId*>(static_cast<void*>(chunk->payload)) + index),
                        *(static_cast<Components*>(static_cast<void*>(chunk->payload + match.offsets[Indices])) +
                          index)...);
                }
            }
        }
    }
} // namespace up
