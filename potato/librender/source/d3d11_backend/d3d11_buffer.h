// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_buffer.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d11 {
    class BufferD3D11 final : public GpuBuffer {
    public:
        BufferD3D11(GpuBufferType type, uint64 size, com_ptr<ID3D11Buffer> buffer) noexcept;
        ~BufferD3D11();

        GpuBufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return _size; }

        com_ptr<ID3D11Buffer> const& buffer() const noexcept { return _buffer; }

    private:
        GpuBufferType _type;
        uint64 _size;
        com_ptr<ID3D11Buffer> _buffer;
    };
} // namespace up::d3d11
