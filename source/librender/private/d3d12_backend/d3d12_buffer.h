// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_buffer.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class DescriptorHeapD3D12;
    class RenderContextD3D12;

    class BufferD3D12 final : public GpuBuffer {
    public:
        BufferD3D12() noexcept;
        ~BufferD3D12() override  = default;

        bool create(RenderContextD3D12 const& ctx, GpuBufferType type, uint64 size); 

        GpuBufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return _size; }

        ID3DResourcePtr const& buffer() const noexcept { return _buffer; }
        D3D12MA::Allocation* const& alloc() const noexcept { return _allocation.get(); }

        D3D12_GPU_DESCRIPTOR_HANDLE getDesc() const { return _desc;  }
        void setDesc(D3D12_GPU_DESCRIPTOR_HANDLE desc) { _desc = desc; }

    private:
        GpuBufferType _type;
        uint64 _size;
        ID3DResourcePtr _buffer;
        com_ptr<D3D12MA::Allocation> _allocation;

        D3D12_GPU_DESCRIPTOR_HANDLE _desc;
    };
} // namespace up::d3d12
