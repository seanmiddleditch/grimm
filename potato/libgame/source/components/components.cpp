// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/demo_components.h"
#include "potato/game/components/camera_component.h"
#include "potato/game/components/mesh_component.h"
#include "potato/game/components/rigidbody_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"

namespace up {
    void registerComponents(Space& space) {
        space.entities().registerComponent<component::Transform>();
        space.entities().registerComponent<component::Camera>();
        space.entities().registerComponent<component::Mesh>();
        space.entities().registerComponent<component::RigidBody>();
        space.entities().registerComponent<component::Wave>();
        space.entities().registerComponent<component::Spin>();
        space.entities().registerComponent<component::Ding>();
    }
} // namespace up
