// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_buffer.h"

#include "potato/spud/int_types.h"

up::d3d11::BufferD3D11::BufferD3D11(GpuBufferType type, up::uint64 size, com_ptr<ID3D11Buffer> buffer) noexcept
    : _type(type)
    , _size(size)
    , _buffer(std::move(buffer)) {}

up::d3d11::BufferD3D11::~BufferD3D11() = default;
