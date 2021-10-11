// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

namespace up {
    class GpuCommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class RenderContext {
    public:
        double frameTime = 0;
        GpuCommandList& commandList;
        GpuDevice& device;
    };
} // namespace up
