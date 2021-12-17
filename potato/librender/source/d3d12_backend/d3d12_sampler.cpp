// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_sampler.h"

up::d3d12::SamplerD3D12::SamplerD3D12() {}

up::d3d12::SamplerD3D12::~SamplerD3D12() = default;

auto up::d3d12::SamplerD3D12::create(const D3D12_GPU_DESCRIPTOR_HANDLE& descriptor) -> bool {
    _descriptor = descriptor;
    return true;
}
