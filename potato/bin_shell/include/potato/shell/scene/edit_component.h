// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/common.h"
#include "potato/reflex/type.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Scene;
    struct SceneComponent;

    class EditComponent {
    public:
        virtual ~EditComponent() = default;

        virtual zstring_view name() const noexcept = 0;
        virtual reflex::TypeInfo const& typeInfo() const noexcept = 0;

        virtual bool syncAdd(Scene& scene, EntityId entityId, SceneComponent const& component) const = 0;
        virtual bool syncUpdate(Scene& scene, EntityId entityId, SceneComponent const& component) const = 0;
        virtual bool syncRemove(Scene& scene, EntityId entityId, SceneComponent const& component) const = 0;

        virtual bool syncGame(Scene& scene, EntityId entityId, SceneComponent const& component) const = 0;
    };
} // namespace up
