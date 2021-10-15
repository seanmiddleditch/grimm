// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "components_schema.h"
#include "scene.h"
#include "scene_schema.h"
#include "scene/edit_component.h"

namespace up {
    struct SceneComponent;

    template <typename T>
    class SimpleEditComponent : public EditComponent {
    public:
        zstring_view name() const noexcept override final { return typeInfo().name; }
        reflex::TypeInfo const& typeInfo() const noexcept override final { return reflex::getTypeInfo<T>(); }
        bool syncAdd(Scene&, EntityId, SceneComponent const&) const override final { return false; }
        bool syncUpdate(Scene&, EntityId, SceneComponent const&) const override final { return false; }
        bool syncRemove(Scene&, EntityId, SceneComponent const&) const override final { return false; }
    };

    class TransformEditComponent : public EditComponent {
    public:
        static auto data(SceneComponent const& component) noexcept -> scene::components::Transform&;

        zstring_view name() const noexcept override { return "Transform"_zsv; }
        reflex::TypeInfo const& typeInfo() const noexcept override {
            return reflex::getTypeInfo<scene::components::Transform>();
        }

        bool syncAdd(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
        bool syncUpdate(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
        bool syncRemove(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
    };

    class MeshEditComponent : public EditComponent {
    public:
        static auto data(SceneComponent const& component) noexcept -> scene::components::Mesh&;

        zstring_view name() const noexcept override { return "Mesh"_zsv; }
        reflex::TypeInfo const& typeInfo() const noexcept override {
            return reflex::getTypeInfo<scene::components::Mesh>();
        }

        bool syncAdd(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
        bool syncUpdate(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
        bool syncRemove(Scene& scene, EntityId entityId, SceneComponent const& component) const override;
    };

    class WaveEditComponent : public SimpleEditComponent<scene::components::Wave> {};
    class SpinEditComponent : public SimpleEditComponent<scene::components::Spin> {};
    class DingEditComponent : public SimpleEditComponent<scene::components::Ding> {};
} // namespace up
