// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"

#include "potato/render/gpu_resource.h"
#include "potato/runtime/com_ptr.h"

namespace up::d3d11 {
    class BufferD3D11 final : public GpuResource {
    public:
        BufferD3D11(GpuBufferType type, uint64 size, com_ptr<ID3D11Buffer> buffer) noexcept;
        ~BufferD3D11();

        GpuResourceType resourceType() const noexcept override { return GpuResourceType::Buffer; }
        GpuBufferType bufferType() const noexcept override { return _type; }
        uint64 size() const noexcept override { return _size; }
        GpuFormat format() const noexcept override { return GpuFormat::Unknown; }
        glm::ivec3 dimensions() const noexcept { return {0, 0, 0}; }

        com_ptr<ID3D11Buffer> const& buffer() const noexcept { return _buffer; }

    private:
        GpuBufferType _type;
        uint64 _size;
        com_ptr<ID3D11Buffer> _buffer;
    };
} // namespace up::d3d11
