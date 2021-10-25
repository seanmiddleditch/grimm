// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "shared_context.h"
#include "world.h"

#include "potato/spud/box.h"
#include "potato/spud/hash.h"
#include "potato/spud/zstring_view.h"

namespace up {

    class ComponentRegistry;
    class World;
    template <typename...>
    class Query;

    /// @brief Manages all of the state and data for all worlds in the ECS implementation
    class Universe {
    public:
        UP_GAME_API Universe();
        UP_GAME_API ~Universe();

        auto createWorld() noexcept -> box<World> { return new_box<World>(_context); }

        template <typename... Components>
        auto createQuery() -> Query<Components...> {
            return Query<Components...>(_context);
        }

        template <typename Component>
        void registerComponent(zstring_view name);

    private:
        UP_GAME_API void _registerComponent(ComponentId componentId, ComponentTypeBase const& componentType);

        rc<EcsSharedContext> _context;
    };

    template <typename Component>
    void Universe::registerComponent(zstring_view name) {
        ComponentId const componentId = makeComponentId<Component>();
        static ComponentType<Component> const componentType;
        _registerComponent(componentId, componentType);
    }
} // namespace up
