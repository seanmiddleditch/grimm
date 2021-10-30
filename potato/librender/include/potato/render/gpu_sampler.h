// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    class GpuSampler {
    public:
        GpuSampler() = default;
        virtual ~GpuSampler() = default;

        GpuSampler(GpuSampler&&) = delete;
        GpuSampler& operator=(GpuSampler&&) = delete;
    };
} // namespace up
