// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "scene_doc.h"
#include "scene/edit_component.h"

#include "potato/game/components/demo_components.h"
#include "potato/game/components/mesh_component.h"
#include "potato/game/components/rigidbody_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/space.h"
#include "potato/schema/scene_schema.h"

namespace up {
    struct SceneComponent;

    template <typename SceneComponentT, typename GameComponentT>
    class SimpleEditComponent : public EditComponent {
    public:
        zstring_view name() const noexcept final { return typeInfo().name; }
        reflex::TypeInfo const& typeInfo() const noexcept final { return reflex::getTypeInfo<SceneComponentT>(); }
        bool syncAdd(Space&, EntityId, SceneComponent const&) const final { return false; }
        bool syncUpdate(Space&, EntityId, SceneComponent const&) const final { return false; }
        bool syncRemove(Space&, EntityId, SceneComponent const&) const final { return false; }

        bool syncGame(Space& space, EntityId entityId, SceneComponent const& component) const final {
            space.entities().addComponent<GameComponentT>(
                entityId,
                createFrom(*static_cast<SceneComponentT*>(component.data.get())));
            return true;
        }

        virtual GameComponentT createFrom(SceneComponentT const& sceneComponent) const = 0;
    };

    class TransformEditComponent : public EditComponent {
    public:
        static auto data(SceneComponent const& component) noexcept -> scene::components::Transform&;

        zstring_view name() const noexcept override { return "Transform"_zsv; }
        reflex::TypeInfo const& typeInfo() const noexcept override {
            return reflex::getTypeInfo<scene::components::Transform>();
        }

        bool syncAdd(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncUpdate(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncRemove(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncGame(Space& space, EntityId entityId, SceneComponent const& component) const override;
    };

    class MeshEditComponent : public EditComponent {
    public:
        static auto data(SceneComponent const& component) noexcept -> scene::components::Mesh&;

        zstring_view name() const noexcept override { return "Mesh"_zsv; }
        reflex::TypeInfo const& typeInfo() const noexcept override {
            return reflex::getTypeInfo<scene::components::Mesh>();
        }

        bool syncAdd(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncUpdate(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncRemove(Space& space, EntityId entityId, SceneComponent const& component) const override;
        bool syncGame(Space& space, EntityId entityId, SceneComponent const& component) const override;
    };

    class WaveEditComponent final : public SimpleEditComponent<scene::components::Wave, DemoWaveComponent> {
        DemoWaveComponent createFrom(scene::components::Wave const& sceneComponent) const override;
    };

    class SpinEditComponent final : public SimpleEditComponent<scene::components::Spin, DemoSpinComponent> {
        DemoSpinComponent createFrom(scene::components::Spin const& sceneComponent) const override;
    };

    class DingEditComponent final : public SimpleEditComponent<scene::components::Ding, DemoDingComponent> {
        DemoDingComponent createFrom(scene::components::Ding const& sceneComponent) const override;
    };

    class BodyEditComponent final : public SimpleEditComponent<scene::components::Body, RigidBodyComponent> {
        RigidBodyComponent createFrom(scene::components::Body const& sceneComponent) const override;
    };

    class TestEditComponent final : public EditComponent {
    public:
        zstring_view name() const noexcept final { return typeInfo().name; }
        reflex::TypeInfo const& typeInfo() const noexcept final {
            return reflex::getTypeInfo<scene::components::Test>();
        }
        bool syncAdd(Space&, EntityId, SceneComponent const&) const final { return false; }
        bool syncUpdate(Space&, EntityId, SceneComponent const&) const final { return false; }
        bool syncRemove(Space&, EntityId, SceneComponent const&) const final { return false; }
        bool syncGame(Space& space, EntityId entityId, SceneComponent const& component) const final { return false; }
    };
} // namespace up
