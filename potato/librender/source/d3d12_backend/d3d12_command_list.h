// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_command_list.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class PipelineStateD3D12;

    class CommandListD3D12 final : public GpuCommandList {
    public:
        CommandListD3D12();
        virtual ~CommandListD3D12();

        CommandListD3D12(CommandListD3D12&&) = delete;
        CommandListD3D12& operator=(CommandListD3D12&&) = delete;

        static box<CommandListD3D12> createCommandList(ID3D12Device* device, GpuPipelineState* pipelineState, D3D12_COMMAND_LIST_TYPE type);

        bool create(ID3D12Device* device, ID3D12PipelineState* pipelineState, D3D12_COMMAND_LIST_TYPE type);

        void setPipelineState(GpuPipelineState* state) override;

        void bindRenderTarget(uint32 index, GpuResourceView* view) override;
        void bindDepthStencil(GpuResourceView* view) override;
        void bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset = 0) override;
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override;
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) override;
        void bindConstantValues(uint32 count, float* values, GpuShaderStage stage) override;
        void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) override;
        void bindTexture(uint32 slot, GpuResourceView* view, GpuSampler* sampler, GpuShaderStage stage) override;
        void setClipRect(GpuClipRect rect) override;

        void setPrimitiveTopology(GpuPrimitiveTopology topology) override;
        void setViewport(GpuViewportDesc const& viewport) override;

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override;
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override;

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override;
        void clearDepthStencil(GpuResourceView* view) override;

        void start(GpuPipelineState* pipelineState) override; 
        void finish() override;
        void clear(GpuPipelineState* pipelineState = nullptr) override;

        span<byte> map(GpuBuffer* buffer, uint64 size, uint64 offset = 0) override;
        void unmap(GpuBuffer* buffer, span<byte const> data) override;
        void update(GpuBuffer* buffer, span<byte const> data, uint64 offset = 0) override;

        void flush(ID3D12CommandQueue* pQueue); 

        ID3DCommandListType* getResource() const { return _commandList.get(); }

    private:
        void _flushBindings();

        static constexpr uint32 maxRenderTargetBindings = 4;

        bool _bindingsDirty = false;
        ID3DCommandAllocatorPtr _commandAllocator;
        ID3DCommandListPtr _commandList;

        // temp stuff to see if I can make this api works 
        PipelineStateD3D12* _pipeline = nullptr; 
    };
} // namespace up::d3d12
