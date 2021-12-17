// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_desc_heap.h"
#include "d3d12_device.h"
#include "d3d12_platform.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

#include <d3d12.h>

up::d3d12::DescriptorHeapD3D12::DescriptorHeapD3D12(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc) {
    create(device, desc);
}

up::d3d12::DescriptorHeapD3D12::DescriptorHeapD3D12(
    ID3D12Device* device,
    uint32 size,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
    create(device, {.Type = type, .NumDescriptors = size, .Flags = flags, .NodeMask = 0});
}

up::d3d12::DescriptorHeapD3D12::~DescriptorHeapD3D12() {
};

auto up::d3d12::DescriptorHeapD3D12::create(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc) -> bool {
    UP_ASSERT(device != nullptr);
    UP_ASSERT(desc.NumDescriptors != 0);
    _desc = desc;

    device->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), out_ptr(_heap));
    if (!_heap) {
        return false;
    }

    _heap->SetName(L"DescriptorHeap");
    _cpu = _heap->GetCPUDescriptorHandleForHeapStart();

    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
        _gpu = _heap->GetGPUDescriptorHandleForHeapStart();
    }
    _increment = device->GetDescriptorHandleIncrementSize(desc.Type);
    return true;
}

auto up::d3d12::DescriptorHeapD3D12::get_cpu(uint64 index) -> D3D12_CPU_DESCRIPTOR_HANDLE{
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = static_cast<SIZE_T>(_cpu.ptr + index * static_cast<uint64>(_increment));
    return handle;
}
auto up::d3d12::DescriptorHeapD3D12::get_gpu(uint64 index) -> D3D12_GPU_DESCRIPTOR_HANDLE {
    D3D12_GPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = static_cast<SIZE_T>(_gpu.ptr + index * static_cast<uint64>(_increment));
    return handle;
}
