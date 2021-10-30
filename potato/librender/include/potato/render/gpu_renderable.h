// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/rc.h"

#include <glm/vec3.hpp>

namespace up {
    class GpuRenderable {
    public:
        GpuRenderable() = default;
        virtual ~GpuRenderable() = default;
    };
} // namespace up
