// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/space.h"
#include "potato/game/world.h"
#include "potato/schema/components_schema.h"

namespace up {
    void registerComponents(Space& space) {
        space.world().registerComponent<components::Transform>();
        space.world().registerComponent<components::Body>();
        space.world().registerComponent<components::Mesh>();
        space.world().registerComponent<components::Wave>();
        space.world().registerComponent<components::Spin>();
        space.world().registerComponent<components::Ding>();
    }
} // namespace up
