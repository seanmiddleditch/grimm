// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_resource_view.h"

namespace up::d3d11 {
    ResourceViewD3D11::ResourceViewD3D11(GpuViewType type, com_ptr<ID3D11View> view)
        : _type(type)
        , _view(std::move(view)) { }

    ResourceViewD3D11::~ResourceViewD3D11() = default;

    void* ResourceViewD3D11::getImguiTexture() const {
        if (_type == GpuViewType::SRV)
            return _view.as<ID3D11ShaderResourceView>().get();
        else
            return nullptr;
    }
} // namespace up::d3d11
