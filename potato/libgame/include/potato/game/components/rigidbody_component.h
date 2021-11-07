// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"

#include <BulletDynamics/Dynamics/btRigidBody.h>

namespace up::component {
    struct RigidBody {
        box<btRigidBody> body;
    };
} // namespace up::component
