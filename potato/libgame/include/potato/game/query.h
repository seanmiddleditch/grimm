// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "component.h"
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

        /// Given a World and a callback and invokes the callback once for each matching entity.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback>
            requires is_invocable_v<Callback, EntityId, Components&...>
        void select(World& world, Callback&& callback);

    private:
        template <typename Callback, size_t... Indices>
        void _execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const;
    };

    template <typename... Components>
    template <typename Callback>
        requires is_invocable_v<Callback, EntityId, Components&...>
    void Query<Components...>::select(World& world, Callback&& callback) {
        _execute(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        ComponentId const componentIds[] = {makeComponentId<Components>()...};
        ComponentStorage* componentStorages[] = {world.getComponentStorage(componentIds[Indices])...};
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
