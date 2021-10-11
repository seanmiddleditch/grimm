// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/rc.h"

#include <glm/vec3.hpp>

namespace up {
    class GpuTexture : public shared<GpuTexture> {
    public:
        GpuTexture() = default;
        virtual ~GpuTexture() = default;

        GpuTexture(GpuTexture&&) = delete;
        GpuTexture& operator=(GpuTexture&&) = delete;

        virtual GpuTextureType type() const noexcept = 0;
        virtual GpuFormat format() const noexcept = 0;
        virtual glm::ivec3 dimensions() const noexcept = 0;
    };
} // namespace up
