// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_command_list.h"
#include "d3d11_buffer.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_platform.h"
#include "d3d11_resource_view.h"
#include "d3d11_sampler.h"
#include "d3d11_texture.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/int_types.h"
#include "potato/spud/out_ptr.h"

up::d3d11::CommandListD3D11::CommandListD3D11(com_ptr<ID3D11DeviceContext> context) : _context(std::move(context)) {}

up::d3d11::CommandListD3D11::~CommandListD3D11() = default;

auto up::d3d11::CommandListD3D11::createCommandList(ID3D11Device* device, GpuPipelineState* pipelineState)
    -> box<CommandListD3D11> {
    com_ptr<ID3D11DeviceContext> context;
    HRESULT hr = device->CreateDeferredContext(0, out_ptr(context));
    if (FAILED(hr) || context == nullptr) {
        return nullptr;
    }

    return new_box<CommandListD3D11>(std::move(context));
}

void up::d3d11::CommandListD3D11::setPipelineState(GpuPipelineState* state) {
    UP_ASSERT(state != nullptr);

    auto pipelineState = static_cast<PipelineStateD3D11*>(state);

    auto const& params = pipelineState->params();

    _context->IASetInputLayout(params.inputLayout.get());
    _context->RSSetState(params.rasterState.get());
    _context->VSSetShader(params.vertShader.get(), nullptr, 0);
    _context->PSSetShader(params.pixelShader.get(), nullptr, 0);
    _context->OMSetBlendState(params.blendState.get(), nullptr, ~UINT(0));
    _context->OMSetDepthStencilState(params.depthStencilState.get(), 0);
}

void up::d3d11::CommandListD3D11::bindRenderTarget(up::uint32 index, GpuResourceView* view) {
    UP_ASSERT(index < maxRenderTargetBindings);

    if (view == nullptr) {
        _rtv[index].reset();
        return;
    }

    auto rtv = static_cast<ResourceViewD3D11*>(view);
    UP_ASSERT(rtv->type() == GpuViewType::RTV);

    _rtv[index] = rtv->getView().as<ID3D11RenderTargetView>();

    _bindingsDirty = true;
}

void up::d3d11::CommandListD3D11::bindDepthStencil(GpuResourceView* view) {
    auto dsv = static_cast<ResourceViewD3D11*>(view);
    UP_ASSERT(dsv->type() == GpuViewType::DSV);

    _dsv = dsv->getView().as<ID3D11DepthStencilView>();
    _bindingsDirty = true;
}

void up::d3d11::CommandListD3D11::bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, up::uint32 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Index);

    auto d3dBuffer = static_cast<BufferD3D11*>(buffer);
    auto* d3d11Buffer = static_cast<ID3D11Buffer*>(d3dBuffer->buffer().get());

    UINT d3dOffset = static_cast<UINT>(offset);
    _context->IASetIndexBuffer(d3d11Buffer, toNative(indexType), d3dOffset);
}

void up::d3d11::CommandListD3D11::bindVertexBuffer(
    up::uint32 slot,
    GpuBuffer* buffer,
    up::uint64 stride,
    up::uint64 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Vertex);

    auto d3dBuffer = static_cast<BufferD3D11*>(buffer);
    auto* d3d11Buffer = static_cast<ID3D11Buffer*>(d3dBuffer->buffer().get());

    UINT d3dStride = static_cast<UINT>(stride);
    UINT d3dOffset = static_cast<UINT>(offset);
    _context->IASetVertexBuffers(slot, 1, &d3d11Buffer, &d3dStride, &d3dOffset);
}

void up::d3d11::CommandListD3D11::bindConstantBuffer(up::uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Constant);

    auto d3dBuffer = static_cast<BufferD3D11*>(buffer);
    auto* d3d11Buffer = static_cast<ID3D11Buffer*>(d3dBuffer->buffer().get());

    if ((uint32(stage) & uint32(GpuShaderStage::Vertex)) != 0) {
        _context->VSSetConstantBuffers(slot, 1, &d3d11Buffer);
    }
    if ((uint32(stage) & uint32(GpuShaderStage::Pixel)) != 0) {
        _context->PSSetConstantBuffers(slot, 1, &d3d11Buffer);
    }
}

void up::d3d11::CommandListD3D11::bindShaderResource(up::uint32 slot, GpuResourceView* view, GpuShaderStage stage) {
    UP_ASSERT(view != nullptr);

    auto buffer = static_cast<ResourceViewD3D11*>(view);
    auto* srv = static_cast<ID3D11ShaderResourceView*>(buffer->getView().get());

    if ((uint32(stage) & uint32(GpuShaderStage::Vertex)) != 0) {
        _context->VSSetShaderResources(slot, 1, &srv);
    }
    if ((uint32(stage) & uint32(GpuShaderStage::Pixel)) != 0) {
        _context->PSSetShaderResources(slot, 1, &srv);
    }
}

void up::d3d11::CommandListD3D11::bindSampler(up::uint32 slot, GpuSampler* sampler, GpuShaderStage stage) {
    UP_ASSERT(sampler != nullptr);

    auto d3dSampler = static_cast<SamplerD3D11*>(sampler);
    auto* nativeSampler = static_cast<ID3D11SamplerState*>(d3dSampler->get().get());

    if ((uint32(stage) & uint32(GpuShaderStage::Vertex)) != 0) {
        _context->VSSetSamplers(slot, 1, &nativeSampler);
    }
    if ((uint32(stage) & uint32(GpuShaderStage::Pixel)) != 0) {
        _context->PSSetSamplers(slot, 1, &nativeSampler);
    }
}

void up::d3d11::CommandListD3D11::setPrimitiveTopology(GpuPrimitiveTopology topology) {
    D3D11_PRIMITIVE_TOPOLOGY primitive = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    switch (topology) {
        case GpuPrimitiveTopology::Triangles:
            primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case GpuPrimitiveTopology::Lines:
            primitive = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }
    _context->IASetPrimitiveTopology(primitive);
}

void up::d3d11::CommandListD3D11::setViewport(GpuViewportDesc const& viewport) {
    D3D11_VIEWPORT d3d11Viewport{};
    d3d11Viewport.TopLeftX = viewport.leftX;
    d3d11Viewport.TopLeftY = viewport.topY;
    d3d11Viewport.Width = viewport.width;
    d3d11Viewport.Height = viewport.height;
    d3d11Viewport.MinDepth = viewport.minDepth;
    d3d11Viewport.MaxDepth = viewport.maxDepth;
    _context->RSSetViewports(1, &d3d11Viewport);
}

void up::d3d11::CommandListD3D11::setClipRect(GpuClipRect rect) {
    D3D11_RECT clipRect{};
    clipRect.left = static_cast<LONG>(rect.left);
    clipRect.top = static_cast<LONG>(rect.top);
    clipRect.right = static_cast<LONG>(rect.right);
    clipRect.bottom = static_cast<LONG>(rect.bottom);
    _context->RSSetScissorRects(1, &clipRect);
}

void up::d3d11::CommandListD3D11::draw(up::uint32 vertexCount, up::uint32 firstVertex) {
    _flushBindings();
    _context->Draw(vertexCount, firstVertex);
}

void up::d3d11::CommandListD3D11::drawIndexed(up::uint32 indexCount, up::uint32 firstIndex, up::uint32 baseIndex) {
    _flushBindings();
    _context->DrawIndexed(indexCount, firstIndex, baseIndex);
}

void up::d3d11::CommandListD3D11::clearRenderTarget(GpuResourceView* view, glm::vec4 color) {
    UP_ASSERT(view != nullptr);

    FLOAT c[4] = {color.x, color.y, color.z, color.w};
    _context->ClearRenderTargetView(
        static_cast<ID3D11RenderTargetView*>(static_cast<ResourceViewD3D11*>(view)->getView().get()),
        c);
}

void up::d3d11::CommandListD3D11::clearDepthStencil(GpuResourceView* view) {
    UP_ASSERT(view != nullptr);

    _context->ClearDepthStencilView(
        static_cast<ID3D11DepthStencilView*>(static_cast<ResourceViewD3D11*>(view)->getView().get()),
        D3D11_CLEAR_DEPTH,
        1.f,
        0);
}

void up::d3d11::CommandListD3D11::finish() {
    _context->FinishCommandList(FALSE, out_ptr(_commands));
}

void up::d3d11::CommandListD3D11::clear(GpuPipelineState* pipelineState) {
    _context->ClearState();
    _context->RSSetScissorRects(0, nullptr);
    _commands.reset();
    if (pipelineState != nullptr) {
        setPipelineState(pipelineState);
    }
}

auto up::d3d11::CommandListD3D11::map(GpuBuffer* buffer, up::uint64 size, up::uint64 offset) -> span<up::byte> {
    if (buffer == nullptr) {
        return {};
    }

    ID3D11Buffer* d3dBuffer = static_cast<BufferD3D11*>(buffer)->buffer().get();
    UP_ASSERT(d3dBuffer != nullptr);

    D3D11_MAPPED_SUBRESOURCE sub = {};

    UP_ASSERT(offset < buffer->size());
    UP_ASSERT(size <= buffer->size() - offset);

    // bool writeAll = offset == 0 && size == buffer->size();

    //_context->Map(d3dBuffer, 0, writeAll ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &sub);
    _context->Map(d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
    return {static_cast<up::byte*>(sub.pData) + offset, static_cast<std::size_t>(size)};
}

void up::d3d11::CommandListD3D11::unmap(GpuBuffer* buffer, span<up::byte const> data) {
    if (buffer == nullptr) {
        return;
    }

    ID3D11Buffer* d3dBuffer = static_cast<BufferD3D11*>(buffer)->buffer().get();

    _context->Unmap(d3dBuffer, 0);
}

void up::d3d11::CommandListD3D11::update(GpuBuffer* buffer, span<up::byte const> data, up::uint64 offset) {
    if (buffer == nullptr) {
        return;
    }

    auto target = map(buffer, data.size(), offset);

    UP_ASSERT(data.size() <= target.size());

    std::memcpy(target.data(), data.data(), data.size());
    unmap(buffer, target);
}

void up::d3d11::CommandListD3D11::_flushBindings() {
    if (_bindingsDirty) {
        _bindingsDirty = false;
        _context->OMSetRenderTargets(
            maxRenderTargetBindings,
            reinterpret_cast<ID3D11RenderTargetView**>(&_rtv),
            _dsv.get());
        for (auto& view : _rtv) {
            view.reset();
        }
        _dsv.reset();
    }
}
