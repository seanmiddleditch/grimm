// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/rc.h"

#include <glm/vec3.hpp>

namespace up {
    class GpuResource : public shared<GpuResource> {
    public:
        GpuResource() = default;
        virtual ~GpuResource() = default;

        GpuResource(GpuResource&&) = delete;
        GpuResource& operator=(GpuResource&&) = delete;

        virtual GpuResourceType resourceType() const noexcept = 0;
        virtual GpuBufferType bufferType() const noexcept = 0;
        virtual uint64 size() const noexcept = 0;
        virtual GpuFormat format() const noexcept = 0;
        virtual glm::ivec3 dimensions() const noexcept = 0;
    };
} // namespace up
