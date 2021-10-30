// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_factory.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d12 {
    // \brief: wrapper for D3D12 descriptor heaps 
    class DescriptorHeapD3D12 {
    public:
        DescriptorHeapD3D12(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);
        DescriptorHeapD3D12(
            ID3D12Device* device,
            uint32 size,
            D3D12_DESCRIPTOR_HEAP_TYPE type,
            D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

        virtual ~DescriptorHeapD3D12();

        D3D12_CPU_DESCRIPTOR_HANDLE get_cpu(uint64 index); 
        D3D12_GPU_DESCRIPTOR_HANDLE get_gpu(uint64 index);

        ID3D12DescriptorHeap* heap() const { return _heap.get(); }
        uint32 increment() const { _increment; }

    protected:
        bool create(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

    private:
        ID3DDescriptorHeapPtr _heap;
        D3D12_DESCRIPTOR_HEAP_DESC _desc;
        D3D12_CPU_DESCRIPTOR_HANDLE _cpu;
        D3D12_GPU_DESCRIPTOR_HANDLE _gpu;
        uint32 _increment = 0;
    };
} // namespace up::d3d12
