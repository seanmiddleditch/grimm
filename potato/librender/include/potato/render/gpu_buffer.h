// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/int_types.h"
#include "potato/spud/span.h"

namespace up {
    class GpuBuffer {
    public:
        GpuBuffer() = default;
        virtual ~GpuBuffer() = default;

        GpuBuffer(GpuBuffer&&) = delete;
        GpuBuffer& operator=(GpuBuffer&&) = delete;

        virtual GpuBufferType type() const noexcept = 0;
        virtual uint64 size() const noexcept = 0;
    };
} // namespace up
