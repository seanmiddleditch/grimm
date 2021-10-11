// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_sampler.h"

up::d3d11::SamplerD3D11::SamplerD3D11(com_ptr<ID3D11SamplerState> sampler) : _sampler(std::move(sampler)) {}

up::d3d11::SamplerD3D11::~SamplerD3D11() = default;
