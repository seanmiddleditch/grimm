// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "edit_components.h"
#include "scene_doc.h"

#include "potato/game/entity_manager.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>

static constexpr glm::quat eulerToQuat(up::schema::Euler const& euler) noexcept {
    float const pitch = glm::radians(euler.pitch);
    float const yaw = glm::radians(euler.yaw);
    float const roll = glm::radians(euler.roll);
    return glm::quat({pitch, yaw, roll});
}

auto up::TransformEditComponent::data(SceneComponent const& component) noexcept -> scene::components::Transform& {
    return *static_cast<scene::components::Transform*>(component.data.get());
}

bool up::TransformEditComponent::syncAdd(Space& space, EntityId entityId, SceneComponent const& component) const {
    auto& trans = space.entities().addComponent<TransformComponent>(entityId);
    trans.position = data(component).position;
    trans.rotation = eulerToQuat(data(component).rotation);
    return true;
}

bool up::TransformEditComponent::syncUpdate(Space& space, EntityId entityId, SceneComponent const& component) const {
    auto& trans = *space.entities().getComponentSlow<TransformComponent>(entityId);
    trans.position = data(component).position;
    trans.rotation = eulerToQuat(data(component).rotation);
    return true;
}

bool up::TransformEditComponent::syncRemove(Space& space, EntityId entityId, SceneComponent const& component) const {
    space.entities().removeComponent<TransformComponent>(entityId);
    return true;
}

bool up::TransformEditComponent::syncGame(Space& space, EntityId entityId, SceneComponent const& component) const {
    return syncAdd(space, entityId, component);
}

auto up::MeshEditComponent::data(SceneComponent const& component) noexcept -> scene::components::Mesh& {
    return *static_cast<scene::components::Mesh*>(component.data.get());
}

bool up::MeshEditComponent::syncAdd(Space& space, EntityId entityId, SceneComponent const& component) const {
    space.entities().addComponent(
        entityId,
        MeshComponent{.mesh = data(component).mesh, .material = data(component).material});
    return true;
}

bool up::MeshEditComponent::syncUpdate(Space& space, EntityId entityId, SceneComponent const& component) const {
    *space.entities().getComponentSlow<MeshComponent>(entityId) =
        MeshComponent{.mesh = data(component).mesh, .material = data(component).material};
    return true;
}

bool up::MeshEditComponent::syncRemove(Space& space, EntityId entityId, SceneComponent const& component) const {
    space.entities().removeComponent<MeshComponent>(entityId);
    return true;
}

bool up::MeshEditComponent::syncGame(Space& space, EntityId entityId, SceneComponent const& component) const {
    return syncAdd(space, entityId, component);
}

auto up::WaveEditComponent::createFrom(scene::components::Wave const& sceneComponent) const -> DemoWaveComponent {
    return {.time = 0, .offset = sceneComponent.offset};
}

auto up::SpinEditComponent::createFrom(scene::components::Spin const& sceneComponent) const -> DemoSpinComponent {
    return {.radians = sceneComponent.radians};
}

auto up::DingEditComponent::createFrom(scene::components::Ding const& sceneComponent) const -> DemoDingComponent {
    return {.period = sceneComponent.period, .time = 0, .sound = sceneComponent.sound};
}

auto up::BodyEditComponent::createFrom(scene::components::Body const& sceneComponent) const -> RigidBodyComponent {
    return {};
}
