// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/common.h"
#include "potato/reflex/type.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Space;
    struct SceneComponent;

    class EditComponent {
    public:
        virtual ~EditComponent() = default;

        virtual zstring_view name() const noexcept = 0;
        virtual reflex::TypeInfo const& typeInfo() const noexcept = 0;

        virtual bool syncAdd(Space& space, EntityId entityId, SceneComponent const& component) const = 0;
        virtual bool syncUpdate(Space& space, EntityId entityId, SceneComponent const& component) const = 0;
        virtual bool syncRemove(Space& space, EntityId entityId, SceneComponent const& component) const = 0;

        virtual bool syncGame(Space& space, EntityId entityId, SceneComponent const& component) const = 0;
    };
} // namespace up
