// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_sampler.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d11 {
    class SamplerD3D11 final : public GpuSampler {
    public:
        explicit SamplerD3D11(com_ptr<ID3D11SamplerState> sampler);
        virtual ~SamplerD3D11();

        SamplerD3D11(SamplerD3D11&&) = delete;
        SamplerD3D11& operator=(SamplerD3D11&&) = delete;

        com_ptr<ID3D11SamplerState> const& get() const noexcept { return _sampler; }

    private:
        com_ptr<ID3D11SamplerState> _sampler;
    };
} // namespace up::d3d11
