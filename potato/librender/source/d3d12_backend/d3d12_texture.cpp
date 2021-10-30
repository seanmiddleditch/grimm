// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_texture.h"
#include "d3d12_command_list.h"
#include "d3d12_context.h"
#include "d3d12_platform.h"
#include "d3d12_utils.h"
#include "d3d12_desc_heap.h"
#include "d3d12_device.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <d3d12.h>

up::d3d12::TextureD3D12::TextureD3D12() {}

up::d3d12::TextureD3D12::TextureD3D12(ID3DResourcePtr buffer) : _texture(buffer) {}


namespace ResourceDesc { 
static inline D3D12_RESOURCE_DESC ResourceDesc(
    D3D12_RESOURCE_DIMENSION dimension,
    UINT64 alignment,
    UINT64 width,
    UINT height,
    UINT16 depthOrArraySize,
    UINT16 mipLevels,
    DXGI_FORMAT format,
    UINT sampleCount,
    UINT sampleQuality,
    D3D12_TEXTURE_LAYOUT layout,
    D3D12_RESOURCE_FLAGS flags) {

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = dimension;
    desc.Alignment = alignment;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = depthOrArraySize;
    desc.MipLevels = mipLevels;
    desc.Format = format;
    desc.SampleDesc.Count = sampleCount;
    desc.SampleDesc.Quality = sampleQuality;
    desc.Layout = layout;
    desc.Flags = flags;
    return desc; 
}

 static inline D3D12_RESOURCE_DESC Buffer(
    UINT64 width,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
    UINT64 alignment = 0) noexcept {
    return ResourceDesc(
        D3D12_RESOURCE_DIMENSION_BUFFER,
        alignment,
        width,
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        flags);
}

 static inline D3D12_RESOURCE_DESC Tex2D(
    DXGI_FORMAT format,
    UINT64 width,
    UINT height,
    UINT16 arraySize = 1,
    UINT16 mipLevels = 0,
    UINT sampleCount = 1,
    UINT sampleQuality = 0,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
    D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
    UINT64 alignment = 0) noexcept {
    return ResourceDesc(
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        alignment,
        width,
        height,
        arraySize,
        mipLevels,
        format,
        sampleCount,
        sampleQuality,
        layout,
        flags);
}
} // namespace ResourceDesc

auto up::d3d12::TextureD3D12::create(RenderContextD3D12 const& ctx, GpuTextureDesc const& desc, span<up::byte const> data)
    -> bool {
    if (desc.type == GpuTextureType::Texture2D) {
        return create2DTex(ctx, desc, data);
    }
    if (desc.type == GpuTextureType::DepthStencil) {
        return createDepthStencilTex(ctx, desc);
    }

     UP_UNREACHABLE("Unsupported texture type");
    return false; 
}

auto up::d3d12::TextureD3D12::create2DTex(RenderContextD3D12 const& ctx, GpuTextureDesc const& desc, span<up::byte const> data)
        ->bool {
    _format = toNative(desc.format);
    D3D12_RESOURCE_DESC resourceDesc = ResourceDesc::Tex2D(_format, desc.width, desc.height, 1, 1);

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;


    ctx.allocator()->CreateResource(
        &allocDesc,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        NULL,
        out_ptr(_allocation),
        __uuidof(ID3D12Resource),
        out_ptr(_texture));

    if (data.size() != 0) {
        uploadData(ctx, resourceDesc, data);
    }

    return true;
}

auto up::d3d12::TextureD3D12::createDepthStencilTex(RenderContextD3D12 const& ctx, GpuTextureDesc const& desc)
        ->bool {
    _format = toNative(desc.format);
    D3D12_RESOURCE_DESC resourceDesc =
        ResourceDesc::Tex2D(_format, desc.width, desc.height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = _format;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    ctx.allocator()->CreateResource(
        &allocDesc,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        out_ptr(_allocation),
        __uuidof(ID3D12Resource),
        out_ptr(_texture));

    return true;
}


auto up::d3d12::TextureD3D12::uploadData(RenderContextD3D12 const& ctx, D3D12_RESOURCE_DESC& resourceDesc, span<up::byte const> data)
    -> bool {

    uint32 stride = static_cast<uint32>(resourceDesc.Width * toByteSize(fromNative(resourceDesc.Format)));

    UINT64 textureUploadBufferSize = 0;
    static_cast<DeviceD3D12*>(ctx.device())->getDevice()->GetCopyableFootprints(
        &resourceDesc,
        0, // FirstSubresource
        1, // NumSubresources
        0, // BaseOffset
        nullptr, // pLayouts
        nullptr, // pNumRows
        nullptr, // pRowSizeInBytes
        &textureUploadBufferSize); // pTotalBytes


    D3D12MA::ALLOCATION_DESC uploadAllocDesc = {};
    uploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC uploadBufferDesc = ResourceDesc::Buffer(textureUploadBufferSize);

    ctx.allocator()->CreateResource(
        &uploadAllocDesc,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, // pOptimizedClearValue
        out_ptr(_uploadAlloc),
        __uuidof(ID3D12Resource),
        out_ptr(_uploadTexture));
    _uploadTexture->SetName(L"textureUpload");

    D3D12_SUBRESOURCE_DATA textureSubresourceData = {};
    textureSubresourceData.pData = data.data();
    textureSubresourceData.RowPitch = stride;
    textureSubresourceData.SlicePitch = stride * resourceDesc.Height;

    UpdateSubresources(
        static_cast<DeviceD3D12*>(ctx.device())->getDevice(),
        ctx.cmdList()->getResource(),
        _texture.get(),
        _uploadTexture.get(),
        0,
        0,
        1,
        &textureSubresourceData);

    D3D12_RESOURCE_BARRIER textureBarrier = {};
    textureBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    textureBarrier.Transition.pResource = _texture.get();
    textureBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    textureBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    textureBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    ctx.cmdList()->getResource()->ResourceBarrier(1, &textureBarrier);

    return true;
}

auto up::d3d12::TextureD3D12::type() const noexcept -> GpuTextureType {
    com_ptr<ID3D12Resource> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D12Resource), out_ptr(texture2D)))) {
        D3D12_RESOURCE_DESC desc = texture2D->GetDesc();
        switch (desc.Dimension) {
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                return GpuTextureType::Texture2D;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                return GpuTextureType::Texture3D;
        }
    }

    UP_UNREACHABLE("could not detect texture type");
    return GpuTextureType::Texture2D;
}

auto up::d3d12::TextureD3D12::format() const noexcept -> GpuFormat {
    return fromNative(nativeFormat());
}

DXGI_FORMAT up::d3d12::TextureD3D12::nativeFormat() const noexcept {
    com_ptr<ID3D12Resource> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D12Resource), out_ptr(texture2D)))) {
        D3D12_RESOURCE_DESC desc = texture2D->GetDesc();
        return desc.Format;
    }
    return DXGI_FORMAT_UNKNOWN;
}

auto up::d3d12::TextureD3D12::dimensions() const noexcept -> glm::ivec3 {
    com_ptr<ID3D12Resource> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D12Resource), out_ptr(texture2D)))) {
        D3D12_RESOURCE_DESC desc = texture2D->GetDesc();
        return {desc.Width, desc.Height, 0};
    }

    UP_UNREACHABLE("could not detect texture type");
    return {0, 0, 0};
}
