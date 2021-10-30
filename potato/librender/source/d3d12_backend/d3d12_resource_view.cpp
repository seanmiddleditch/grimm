// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_resource_view.h"
#include "d3d12_desc_heap.h"

up::d3d12::ResourceViewD3D12::ResourceViewD3D12(GpuViewType type)
    : _type(type)
{}

up::d3d12::ResourceViewD3D12::~ResourceViewD3D12() = default;

void up::d3d12::ResourceViewD3D12::create(DescriptorHeapD3D12* heap, uint32 index) {
    _heap = heap;
    _index = index; 
}

auto up::d3d12::ResourceViewD3D12::getCpuDesc() const -> D3D12_CPU_DESCRIPTOR_HANDLE{
    UP_ASSERT(_heap);
    return _heap->get_cpu(_index);
}

auto up::d3d12::ResourceViewD3D12::getGpuDesc() const -> D3D12_GPU_DESCRIPTOR_HANDLE {
    UP_ASSERT(_heap);
    return _heap->get_gpu(_index);
}
