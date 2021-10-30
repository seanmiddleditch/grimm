// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.


#include "d3d12_context.h"
#include "d3d12_device.h"
#include "d3d12_command_list.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/span.h"

void up::d3d12::RenderContextD3D12::setPipelineState(GpuPipelineState* state) {
    _cmdList->setPipelineState(state);
}

void up::d3d12::RenderContextD3D12::bindRenderTarget(uint32 index, GpuResourceView* view){
    _cmdList->bindRenderTarget(index, view); 
}

void up::d3d12::RenderContextD3D12::bindDepthStencil(GpuResourceView* view){
    _cmdList->bindDepthStencil(view);
}

void up::d3d12::RenderContextD3D12::bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset){
    _cmdList->bindIndexBuffer(buffer, indexType, offset);
}

void up::d3d12::RenderContextD3D12::bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset){
    _cmdList->bindVertexBuffer(slot, buffer, stride, offset);
}

void up::d3d12::RenderContextD3D12::bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) {
    _cmdList->bindConstantBuffer(slot, buffer, stage);
}

void up::d3d12::RenderContextD3D12::bindConstantValues(uint32 count, float* values, GpuShaderStage stage) {
    _cmdList->bindConstantValues(count, values, stage);
}

void up::d3d12::RenderContextD3D12::bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) {
    _cmdList->bindShaderResource(slot, view, stage);
}

void up::d3d12::RenderContextD3D12::bindTexture(uint32 slot, GpuResourceView* view, GpuSampler* sampler, GpuShaderStage stage) {
    _cmdList->bindTexture(slot, view, sampler, stage);
}

void up::d3d12::RenderContextD3D12::setPrimitiveTopology(GpuPrimitiveTopology topology) {
    _cmdList->setPrimitiveTopology(topology);
}

void up::d3d12::RenderContextD3D12::setViewport(GpuViewportDesc const& viewport) {
    _cmdList->setViewport(viewport);
}

void up::d3d12::RenderContextD3D12::setClipRect(GpuClipRect rect) {
    _cmdList->setClipRect(rect);
}

void up::d3d12::RenderContextD3D12::draw(uint32 vertexCount, uint32 firstVertex) {
    _cmdList->draw(vertexCount, firstVertex);
}

void up::d3d12::RenderContextD3D12::drawIndexed(uint32 indexCount, uint32 firstIndex, uint32 baseIndex) {
    _cmdList->drawIndexed(indexCount, firstIndex, baseIndex);
}

void up::d3d12::RenderContextD3D12::clearRenderTarget(GpuResourceView* view, glm::vec4 color) {
    _cmdList->clearRenderTarget(view, color);
}

void up::d3d12::RenderContextD3D12::clearDepthStencil(GpuResourceView* view) {
    _cmdList->clearDepthStencil(view);
}

void up::d3d12::RenderContextD3D12::start(GpuPipelineState* pipelineState) {
    _cmdList->start(pipelineState);
}

void up::d3d12::RenderContextD3D12::finish() {
    _cmdList->finish();
}

void up::d3d12::RenderContextD3D12::clear(GpuPipelineState* pipelineState) {
    _cmdList->clear(pipelineState);
}

up::span<up::byte> up::d3d12::RenderContextD3D12::map(GpuBuffer* resource, uint64 size, uint64 offset) {
    return _cmdList->map(resource, size, offset);
}

void up::d3d12::RenderContextD3D12::unmap(GpuBuffer* resource, up::span<up::byte const> data) {
    _cmdList->unmap(resource, data);
}

void up::d3d12::RenderContextD3D12::update(GpuBuffer* resource, up::span<up::byte const> data, uint64 offset) {
    _cmdList->update(resource, data, offset);
}

auto up::d3d12::RenderContextD3D12::device() const -> GpuDevice* {
    return _device;
}
