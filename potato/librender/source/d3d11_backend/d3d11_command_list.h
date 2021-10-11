// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_command_list.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d11 {
    class CommandListD3D11 final : public GpuCommandList {
    public:
        CommandListD3D11(com_ptr<ID3D11DeviceContext> context);
        virtual ~CommandListD3D11();

        CommandListD3D11(CommandListD3D11&&) = delete;
        CommandListD3D11& operator=(CommandListD3D11&&) = delete;

        static box<CommandListD3D11> createCommandList(ID3D11Device* device, GpuPipelineState* pipelineState);

        void setPipelineState(GpuPipelineState* state) override;

        void bindRenderTarget(uint32 index, GpuResourceView* view) override;
        void bindDepthStencil(GpuResourceView* view) override;
        void bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset = 0) override;
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override;
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) override;
        void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) override;
        void bindSampler(uint32 slot, GpuSampler* sampler, GpuShaderStage stage) override;
        void setClipRect(GpuClipRect rect) override;

        void setPrimitiveTopology(GpuPrimitiveTopology topology) override;
        void setViewport(GpuViewportDesc const& viewport) override;

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override;
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override;

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override;
        void clearDepthStencil(GpuResourceView* view) override;

        void finish() override;
        void clear(GpuPipelineState* pipelineState = nullptr) override;

        span<byte> map(GpuBuffer* buffer, uint64 size, uint64 offset = 0) override;
        void unmap(GpuBuffer* buffer, span<byte const> data) override;
        void update(GpuBuffer* buffer, span<byte const> data, uint64 offset = 0) override;

        com_ptr<ID3D11DeviceContext> const& deviceContext() const { return _context; }
        com_ptr<ID3D11CommandList> const& commandList() const { return _commands; }

    private:
        void _flushBindings();

        static constexpr uint32 maxRenderTargetBindings = 4;

        com_ptr<ID3D11DeviceContext> _context;
        com_ptr<ID3D11RenderTargetView> _rtv[maxRenderTargetBindings];
        com_ptr<ID3D11DepthStencilView> _dsv;
        com_ptr<ID3D11CommandList> _commands;
        bool _bindingsDirty = false;
    };
} // namespace up::d3d11
