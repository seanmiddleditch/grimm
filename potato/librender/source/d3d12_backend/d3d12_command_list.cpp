// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_command_list.h"
#include "d3d12_buffer.h"
#include "d3d12_desc_heap.h"
#include "d3d12_pipeline_state.h"
#include "d3d12_platform.h"
#include "d3d12_resource_view.h"
#include "d3d12_sampler.h"
#include "d3d12_texture.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/int_types.h"
#include "potato/spud/out_ptr.h"

up::d3d12::CommandListD3D12::CommandListD3D12() { }

up::d3d12::CommandListD3D12::~CommandListD3D12() = default;

auto up::d3d12::CommandListD3D12::createCommandList(
    ID3D12Device* device,
    GpuPipelineState* pipelineState,
    D3D12_COMMAND_LIST_TYPE type) -> box<CommandListD3D12> {
    auto cl = new_box<CommandListD3D12>();
    auto state = pipelineState != nullptr ? static_cast<PipelineStateD3D12*>(pipelineState)->state() : nullptr;
    cl->create(device, state, type);
    return std::move(cl);
}

auto up::d3d12::CommandListD3D12::create(
    ID3D12Device* device,
    ID3D12PipelineState* pipelineState,
    D3D12_COMMAND_LIST_TYPE type) -> bool {
    device->CreateCommandAllocator(type, __uuidof(ID3D12CommandAllocator), out_ptr(_commandAllocator));

    _commandAllocator->SetName(L"CommandAllocator");

    HRESULT hr = device->CreateCommandList(
        0,
        type,
        _commandAllocator.get(),
        pipelineState,
        __uuidof(ID3D12GraphicsCommandList),
        out_ptr(_commandList));

    if (FAILED(hr)) {
        return false;
    }
    _commandList->SetName(L"CommandList");
    _commandList->Close();
    return true;
}

void up::d3d12::CommandListD3D12::setPipelineState(GpuPipelineState* state) {
    UP_ASSERT(state != nullptr);
    _pipeline = static_cast<PipelineStateD3D12*>(state);
    _pipeline->bindPipeline(_commandList.get());
}

void up::d3d12::CommandListD3D12::bindRenderTarget(up::uint32 index, GpuResourceView* view) {
    UP_ASSERT(index < maxRenderTargetBindings);
    auto srv = static_cast<ResourceViewD3D12*>(view);
    auto desc = srv->getCpuDesc();
    _commandList->OMSetRenderTargets(1, &desc, FALSE, nullptr);
}

void up::d3d12::CommandListD3D12::bindDepthStencil(GpuResourceView* view) {
    auto dsv = static_cast<ResourceViewD3D12*>(view);
    UP_ASSERT(dsv->type() == GpuViewType::DSV);
}

void up::d3d12::CommandListD3D12::bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, up::uint32 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Index);

    auto impl = static_cast<BufferD3D12*>(buffer);
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = impl->buffer()->GetGPUVirtualAddress() + offset;
    view.Format = toNative(indexType);
    view.SizeInBytes = static_cast<UINT>(impl->size());

    _commandList->IASetIndexBuffer(&view);
}

void up::d3d12::CommandListD3D12::bindVertexBuffer(
    up::uint32 slot,
    GpuBuffer* buffer,
    up::uint64 stride,
    up::uint64 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Vertex);

    auto impl = static_cast<BufferD3D12*>(buffer);
    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = impl->buffer()->GetGPUVirtualAddress() + offset;
    view.StrideInBytes = static_cast<UINT>(stride);
    view.SizeInBytes = static_cast<UINT>(impl->size());

    _commandList->IASetVertexBuffers(slot, 1, &view);
}

void up::d3d12::CommandListD3D12::bindConstantBuffer(up::uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Constant);

    auto cb = static_cast<BufferD3D12*>(buffer);

    _pipeline->bindConstBuffer(_commandList.get(), slot, cb->buffer()->GetGPUVirtualAddress());
}

void up::d3d12::CommandListD3D12::bindConstantValues(up::uint32 count, float* values, GpuShaderStage stage) {
    UP_ASSERT(values != nullptr);
    _pipeline->bindConstValues(_commandList.get(), count, values);
}

void up::d3d12::CommandListD3D12::bindShaderResource(up::uint32 slot, GpuResourceView* view, GpuShaderStage stage) {
    UP_ASSERT(view != nullptr);
}

void up::d3d12::CommandListD3D12::bindTexture(
    up::uint32 slot,
    GpuResourceView* view,
    GpuSampler* sampler,
    GpuShaderStage stage) {
    UP_ASSERT(view != nullptr);
    UP_ASSERT(sampler != nullptr);

    auto srv = static_cast<ResourceViewD3D12*>(view);
    auto s = static_cast<SamplerD3D12*>(sampler);

    _pipeline->bindTexture(_commandList.get(), slot, srv->getGpuDesc(), s->desc());
}

void up::d3d12::CommandListD3D12::setPrimitiveTopology(GpuPrimitiveTopology topology) {
    D3D12_PRIMITIVE_TOPOLOGY primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // default
    switch (topology) {
        case GpuPrimitiveTopology::Lines:
            primitive = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }
    _commandList->IASetPrimitiveTopology(primitive);
}

void up::d3d12::CommandListD3D12::setViewport(GpuViewportDesc const& viewport) {
    D3D12_VIEWPORT d3d12Viewport;
    d3d12Viewport.TopLeftX = viewport.leftX;
    d3d12Viewport.TopLeftY = viewport.topY;
    d3d12Viewport.Width = viewport.width;
    d3d12Viewport.Height = viewport.height;
    d3d12Viewport.MinDepth = viewport.minDepth;
    d3d12Viewport.MaxDepth = viewport.maxDepth;
    _commandList->RSSetViewports(1, &d3d12Viewport);
}

void up::d3d12::CommandListD3D12::setClipRect(GpuClipRect rect) {
    D3D12_RECT clipRect;
    clipRect.left = static_cast<LONG>(rect.left);
    clipRect.top = static_cast<LONG>(rect.top);
    clipRect.right = static_cast<LONG>(rect.right);
    clipRect.bottom = static_cast<LONG>(rect.bottom);
    _commandList->RSSetScissorRects(1, &clipRect);
}

void up::d3d12::CommandListD3D12::draw(up::uint32 vertexCount, up::uint32 firstVertex) {
    _commandList->DrawInstanced(vertexCount, 1, firstVertex, 0);
}

void up::d3d12::CommandListD3D12::drawIndexed(up::uint32 indexCount, up::uint32 firstIndex, up::uint32 baseIndex) {
    _commandList->DrawIndexedInstanced(indexCount, 1, firstIndex, baseIndex, 0);
}

void up::d3d12::CommandListD3D12::clearRenderTarget(GpuResourceView* view, glm::vec4 color) {
    UP_ASSERT(view != nullptr);
    auto rtv = static_cast<ResourceViewD3D12*>(view);

    float clearColor[4] = {color.x, color.y, color.z, color.w};
    _commandList->ClearRenderTargetView(rtv->getCpuDesc(), clearColor, 0, nullptr);
}

void up::d3d12::CommandListD3D12::clearDepthStencil(GpuResourceView* view) {
    UP_ASSERT(view != nullptr);
    auto dsv = static_cast<ResourceViewD3D12*>(view);
    _commandList->ClearDepthStencilView(dsv->getCpuDesc(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void up::d3d12::CommandListD3D12::start(GpuPipelineState* pipelineState) {
    auto ps = static_cast<PipelineStateD3D12*>(pipelineState);
    _commandAllocator->Reset();
    _commandList->Reset(_commandAllocator.get(), ps ? ps->state() : nullptr);
}

void up::d3d12::CommandListD3D12::finish() {
    if (FAILED(_commandList->Close())) { }
}

void up::d3d12::CommandListD3D12::clear(GpuPipelineState* pipelineState) {
    auto ps = static_cast<PipelineStateD3D12*>(pipelineState);
    _commandList->ClearState(ps ? ps->state() : nullptr);
    _commandList->RSSetScissorRects(0, nullptr);
    if (pipelineState != nullptr) {
        setPipelineState(pipelineState);
    }
}

auto up::d3d12::CommandListD3D12::map(GpuBuffer* buffer, up::uint64 size, up::uint64 offset) -> span<up::byte> {
    if (buffer == nullptr) {
        return {};
    }
    auto d3dBuffer = static_cast<BufferD3D12*>(buffer);

    UP_ASSERT(offset < buffer->size());
    UP_ASSERT(size <= buffer->size() - offset);

    D3D12_RANGE readRange = {0, 0}; // We do not intend to read from this resource on the CPU.
    up::byte* data = nullptr;
    d3dBuffer->buffer()->Map(0, &readRange, reinterpret_cast<void**>(&data));
    return {static_cast<up::byte*>(data) + offset, static_cast<std::size_t>(size)};
}

void up::d3d12::CommandListD3D12::unmap(GpuBuffer* buffer, span<up::byte const> data) {
    if (buffer == nullptr) {
        return;
    }

    auto d3dBuffer = static_cast<BufferD3D12*>(buffer);
    d3dBuffer->buffer()->Unmap(0, nullptr);
}

void up::d3d12::CommandListD3D12::update(GpuBuffer* buffer, span<up::byte const> data, up::uint64 offset) {
    if (buffer == nullptr) {
        return;
    }

    auto target = map(buffer, data.size(), offset);

    UP_ASSERT(data.size() <= target.size());

    std::memcpy(target.data(), data.data(), data.size());
    unmap(buffer, target);
}

void up::d3d12::CommandListD3D12::_flushBindings() { }
