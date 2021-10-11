// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "shared_context.h"
#include "world.h"

#include "potato/reflex/schema.h"
#include "potato/reflex/type.h"
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
        UP_ECS_API Universe();
        UP_ECS_API ~Universe();

        auto createWorld() noexcept -> World { return World(_context); }

        template <typename... Components>
        auto createQuery() -> Query<Components...> {
            return Query<Components...>(_context);
        }

        template <typename Component>
        void registerComponent(zstring_view name);

        UP_ECS_API reflex::TypeInfo const* findComponentByName(string_view name) const noexcept;

        auto components() const noexcept -> view<reflex::TypeInfo const*> { return _context->components; }

    private:
        UP_ECS_API void _registerComponent(reflex::TypeInfo const& typeInfo);

        rc<EcsSharedContext> _context;
    };

    template <typename Component>
    void Universe::registerComponent(zstring_view name) {
        static const reflex::TypeInfo typeInfo = reflex::makeTypeInfo<Component>(name, &reflex::getSchema<Component>());
        _registerComponent(typeInfo);
    }
} // namespace up
