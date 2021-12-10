// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/demo_components.h"
#include "potato/game/components/camera_component.h"
#include "potato/game/components/camera_controllers.h"
#include "potato/game/components/mesh_component.h"
#include "potato/game/components/rigidbody_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"

namespace up {
    void registerComponents(Space& space) {
        space.entities().registerComponent<TransformComponent>();
        space.entities().registerComponent<CameraComponent>();
        space.entities().registerComponent<FlyCameraComponent>();
        space.entities().registerComponent<MeshComponent>();
        space.entities().registerComponent<RigidBodyComponent>();
        space.entities().registerComponent<DemoWaveComponent>();
        space.entities().registerComponent<DemoSpinComponent>();
        space.entities().registerComponent<DemoDingComponent>();
    }
} // namespace up
