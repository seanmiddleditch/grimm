// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_buffer.h"
#include "d3d12_desc_heap.h"

#include "potato/spud/int_types.h"
#include "potato/spud/out_ptr.h"

#include <D3D12MemAlloc.h>

up::d3d12::BufferD3D12::BufferD3D12() noexcept { }

auto up::d3d12::BufferD3D12::create(D3D12MA::Allocator& allocator, GpuBufferType type, uint64 size) -> bool {
    // const buffers have to be multiples of 256
    if (type == GpuBufferType::Constant) {
        size = 256 * ((size / 256) + 1);
    }
    D3D12MA::ALLOCATION_DESC vertexBufferAllocDesc = {};
    vertexBufferAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC vertexBufferResourceDesc = {};
    vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferResourceDesc.Alignment = 0;
    vertexBufferResourceDesc.Width = size;
    vertexBufferResourceDesc.Height = 1;
    vertexBufferResourceDesc.DepthOrArraySize = 1;
    vertexBufferResourceDesc.MipLevels = 1;
    vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferResourceDesc.SampleDesc.Count = 1;
    vertexBufferResourceDesc.SampleDesc.Quality = 0;
    vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    allocator.CreateResource(
        &vertexBufferAllocDesc,
        &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        out_ptr(_allocation),
        __uuidof(ID3D12Resource),
        out_ptr(_buffer));

    _size = size;
    _type = type;

    return true;
};
