// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/rc.h"

#include "gpu_common.h"

namespace up {
    class GpuPipelineLayout : public shared<GpuPipelineLayout> {
    public:
        GpuPipelineLayout() = default;
        virtual ~GpuPipelineLayout() = default;

        GpuPipelineLayout(GpuPipelineLayout&&) = delete;
        GpuPipelineLayout& operator=(GpuPipelineLayout&&) = delete;
    };

    class GpuPipelineState : public shared<GpuPipelineState> {
    public:
        GpuPipelineState() = default;
        virtual ~GpuPipelineState() = default;

        GpuPipelineState(GpuPipelineState&&) = delete;
        GpuPipelineState& operator=(GpuPipelineState&&) = delete;
    };
} // namespace up
