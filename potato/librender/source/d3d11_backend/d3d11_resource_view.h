// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_resource_view.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d11 {
    class ResourceViewD3D11 final : public GpuResourceView {
    public:
        explicit ResourceViewD3D11(GpuViewType type, com_ptr<ID3D11View> view);
        virtual ~ResourceViewD3D11();

        ResourceViewD3D11(ResourceViewD3D11&&) = delete;
        ResourceViewD3D11& operator=(ResourceViewD3D11&&) = delete;

        GpuViewType type() const override { return _type; }
        com_ptr<ID3D11View> const& getView() const { return _view; }

    private:
        GpuViewType _type;
        com_ptr<ID3D11View> _view;
    };
} // namespace up::d3d11
