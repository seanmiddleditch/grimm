// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

namespace up {
    class GpuResourceView {
    public:
        GpuResourceView() = default;
        virtual ~GpuResourceView() = default;

        GpuResourceView(GpuResourceView&&) = delete;
        GpuResourceView& operator=(GpuResourceView&&) = delete;

        virtual GpuViewType type() const = 0;
    };
} // namespace up
