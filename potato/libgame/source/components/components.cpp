// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/space.h"
#include "potato/game/entity_manager.h"
#include "potato/schema/components_schema.h"

namespace up {
    void registerComponents(Space& space) {
        space.entities().registerComponent<components::Transform>();
        space.entities().registerComponent<components::Body>();
        space.entities().registerComponent<components::Mesh>();
        space.entities().registerComponent<components::Wave>();
        space.entities().registerComponent<components::Spin>();
        space.entities().registerComponent<components::Ding>();
    }
} // namespace up
