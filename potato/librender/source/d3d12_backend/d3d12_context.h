// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "context.h"

namespace up {
    class GpuCommandList;
}

namespace up::d3d12 {

    class DeviceD3D12;
    class CommandListD3D12; 

    // container for all common data passed around dx12 api
    class RenderContextD3D12 : public RenderContext {
    public:
        RenderContextD3D12(DeviceD3D12* device, CommandListD3D12* cmdList, D3D12MA::Allocator* allocator)
            : _device(device)
            , _cmdList(cmdList)
            , _allocator(allocator)
        {};

        void setPipelineState(GpuPipelineState* state) override;
        void bindRenderTarget(uint32 index, GpuResourceView* view) override; 
        void bindDepthStencil(GpuResourceView* view) override; 
        void bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset = 0) override; 
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override; 
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) override; 
        void bindConstantValues(uint32 count, float* values, GpuShaderStage stage) override; 
        void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) override; 
        void bindTexture(uint32 slot, GpuResourceView* view, GpuSampler* sampler, GpuShaderStage stage) override; 
        void setPrimitiveTopology(GpuPrimitiveTopology topology) override; 
        void setViewport(GpuViewportDesc const& viewport) override; 
        void setClipRect(GpuClipRect rect) override; 

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override; 
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override; 

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override; 
        void clearDepthStencil(GpuResourceView* view) override; 

        void start(GpuPipelineState* pipelineState) override; 
        void finish() override; 
        void clear(GpuPipelineState* pipelineState = nullptr) override; 

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override; 
        void unmap(GpuBuffer* resource, span<byte const> data) override; 
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override;

        // read-only access to member variables
        GpuDevice* device() const override;
        D3D12MA::Allocator* allocator() const { return _allocator; }
        CommandListD3D12* cmdList() const { return _cmdList; }

    private: 
        DeviceD3D12* _device;
        CommandListD3D12* _cmdList;
        D3D12MA::Allocator* _allocator; 
    };

} // namespace up::d3d12
