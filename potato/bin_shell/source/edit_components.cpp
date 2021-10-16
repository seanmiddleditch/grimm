// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "edit_components.h"
#include "scene_doc.h"

#include "potato/game/world.h"

auto up::TransformEditComponent::data(SceneComponent const& component) noexcept -> scene::components::Transform& {
    return *static_cast<scene::components::Transform*>(component.data.get());
}

bool up::TransformEditComponent::syncAdd(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    scene.world().addComponent(
        entityId,
        components::Transform{.position = data(component).position, .rotation = data(component).rotation});
    return true;
}

bool up::TransformEditComponent::syncUpdate(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    *scene.world().getComponentSlow<components::Transform>(entityId) =
        components::Transform{.position = data(component).position, .rotation = data(component).rotation};
    return true;
}

bool up::TransformEditComponent::syncRemove(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    scene.world().removeComponent<components::Transform>(entityId);
    return true;
}

bool up::TransformEditComponent::syncGame(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    return syncAdd(scene, entityId, component);
}

auto up::MeshEditComponent::data(SceneComponent const& component) noexcept -> scene::components::Mesh& {
    return *static_cast<scene::components::Mesh*>(component.data.get());
}

bool up::MeshEditComponent::syncAdd(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    scene.world().addComponent(
        entityId,
        components::Mesh{.mesh = data(component).mesh, .material = data(component).material});
    return true;
}

bool up::MeshEditComponent::syncUpdate(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    *scene.world().getComponentSlow<components::Mesh>(entityId) =
        components::Mesh{.mesh = data(component).mesh, .material = data(component).material};
    return true;
}

bool up::MeshEditComponent::syncRemove(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    scene.world().removeComponent<components::Mesh>(entityId);
    return true;
}

bool up::MeshEditComponent::syncGame(Scene& scene, EntityId entityId, SceneComponent const& component) const {
    return syncAdd(scene, entityId, component);
}

auto up::WaveEditComponent::createFrom(scene::components::Wave const& sceneComponent) const -> components::Wave {
    return {.time = 0, .offset = sceneComponent.offset};
}

auto up::SpinEditComponent::createFrom(scene::components::Spin const& sceneComponent) const -> components::Spin {
    return {.radians = sceneComponent.radians};
}

auto up::DingEditComponent::createFrom(scene::components::Ding const& sceneComponent) const -> components::Ding {
    return {.period = sceneComponent.period, .time = 0, .sound = sceneComponent.sound};
}
