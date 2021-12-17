// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/rc.h"

namespace up {
    class GpuSampler : public shared<GpuSampler> {
    public:
        GpuSampler() = default;
        virtual ~GpuSampler() = default;
        
        GpuSampler(GpuSampler&&) = delete;
        GpuSampler& operator=(GpuSampler&&) = delete;
    };
} // namespace up
