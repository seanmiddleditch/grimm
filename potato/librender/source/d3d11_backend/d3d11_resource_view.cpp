// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_resource_view.h"

up::d3d11::ResourceViewD3D11::ResourceViewD3D11(GpuViewType type, com_ptr<ID3D11View> view)
    : _type(type)
    , _view(std::move(view)) {}

up::d3d11::ResourceViewD3D11::~ResourceViewD3D11() = default;
