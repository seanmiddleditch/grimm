// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/render/debug_draw.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_sampler.h"
#include "potato/render/gpu_swap_chain.h"

namespace up::null {
    class DeviceNull;

    class FactoryNull final : public GpuDeviceFactory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) override;
        rc<GpuDevice> createDevice(int index) override;
    };

    class DeviceNull final : public GpuDevice {
    public:
        rc<GpuSwapChain> createSwapChain(void* nativeWindow) override;
        rc<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        rc<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        rc<GpuResource> createBuffer(GpuBufferDesc const& desc, GpuDataDesc const& data) override;
        rc<GpuResource> createTexture2D(GpuTextureDesc const& desc, GpuDataDesc const& data) override;
        rc<GpuSampler> createSampler(GpuSamplerDesc const& desc) override;

        box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuResource* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuResource* resource) override;

        void beginImguiFrame(ImGuiContext& context) override;
        void renderImgui(ImGuiContext& context, GpuCommandList& commandList) override;
        void renderDebugDraw(GpuCommandList& commandList) override;

        void execute(GpuCommandList* commands) override { }
    };

    class ResourceViewNull final : public GpuResourceView {
    public:
        ResourceViewNull(GpuViewType type) : _type(type) { }

        GpuViewType type() const override { return _type; }

    private:
        GpuViewType _type;
    };

    class SwapChainNull final : public GpuSwapChain {
    public:
        void present() override { }
        void resizeBuffers(int width, int height) override { }
        rc<GpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class PipelineStateNull final : public GpuPipelineState { };

    class CommandListNull final : public GpuCommandList {
    public:
        void setPipelineState(GpuPipelineState* state) override { }

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override { }
        void clearDepthStencil(GpuResourceView* view) override { }

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override { }
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override { }

        void begin(GpuPipelineState* = nullptr) override { }
        void finish() override { }

        span<byte> map(GpuResource* resource, uint64 size, uint64 offset = 0) override { return {}; }
        void unmap(GpuResource* resource, span<byte const> data) override { }
        void update(GpuResource* resource, span<byte const> data, uint64 offset = 0) override { }

        void bindRenderTargets(span<GpuResourceView* const> renderTargets, GpuResourceView* depthStencil) override { }
        void bindIndexBuffer(GpuResource* buffer, GpuIndexFormat indexType, uint32 offset = 0) override { }
        void bindVertexBuffer(uint32 slot, GpuResource* buffer, uint64 stride, uint64 offset = 0) override { }
        void bindConstantBuffer(uint32 slot, GpuResource* buffer, GpuShaderStage stage) override { }
        void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) override { }
        void bindSampler(uint32 slot, GpuSampler* sampler, GpuShaderStage stage) override { }
        void setPrimitiveTopology(GpuPrimitiveTopology topology) override { }
        void setViewport(GpuViewportDesc const& viewport) override { }
        void setClipRect(GpuClipRect rect) override { }
    };

    class BufferNull final : public GpuResource {
    public:
        BufferNull(GpuBufferType type) : _type(type) { }

        GpuResourceType resourceType() const noexcept override { return GpuResourceType::Buffer; }
        GpuBufferType bufferType() const noexcept override { return _type; }
        uint64 size() const noexcept override { return 0; }
        GpuTextureType textureType() const noexcept override { return GpuTextureType::Texture2D; }
        GpuFormat format() const noexcept override { return GpuFormat::Unknown; }
        glm::ivec3 dimensions() const noexcept override { return {1, 1, 0}; }

    private:
        GpuBufferType _type;
    };

    class TextureNull final : public GpuResource {
    public:
        GpuResourceType resourceType() const noexcept override { return GpuResourceType::Texture; }
        GpuBufferType bufferType() const noexcept override { return GpuBufferType::Constant; }
        uint64 size() const noexcept override { return 0; }
        GpuTextureType textureType() const noexcept override { return GpuTextureType::Texture2D; }
        GpuFormat format() const noexcept override { return GpuFormat::Unknown; }
        glm::ivec3 dimensions() const noexcept override { return {1, 1, 0}; }
    };

    class SamplerNull final : public GpuSampler { };
} // namespace up::null
