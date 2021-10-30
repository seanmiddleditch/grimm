// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_resource_view.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class DescriptorHeapD3D12; 
    class ResourceViewD3D12 final : public GpuResourceView {
    public:
        explicit ResourceViewD3D12(GpuViewType type);
        virtual ~ResourceViewD3D12();

        ResourceViewD3D12(ResourceViewD3D12&&) = delete;
        ResourceViewD3D12& operator=(ResourceViewD3D12&&) = delete;

        void create(DescriptorHeapD3D12* heap, uint32 index);

        GpuViewType type() const override { return _type; }

        DescriptorHeapD3D12* heap() { return _heap; }

        D3D12_CPU_DESCRIPTOR_HANDLE getCpuDesc() const;
        D3D12_GPU_DESCRIPTOR_HANDLE getGpuDesc() const;

    private:
        GpuViewType _type;
        DescriptorHeapD3D12* _heap = nullptr;
        uint32 _index = 0;
    };
} // namespace up::d3d12
