// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_factory.h"
#include "gpu_pipeline_state.h"
#include "gpu_resource_view.h"
#include "gpu_sampler.h"
#include "gpu_swap_chain.h"
#include "gpu_texture.h"

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
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) override;
        rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        box<GpuResourceView> createRenderTargetView(GpuTexture* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuTexture* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuBuffer* buffer) override;
        box<GpuResourceView> createShaderResourceView(GpuTexture* texture) override;

        view<unsigned char> getDebugShader(GpuShaderStage stage) override;

        void execute(GpuCommandList* commands) override {}

        void registerAssetBackends(AssetLoader& assetLoader) override {}
    };

    class ResourceViewNull final : public GpuResourceView {
    public:
        ResourceViewNull(GpuViewType type) : _type(type) {}

        GpuViewType type() const override { return _type; }

    private:
        GpuViewType _type;
    };

    class SwapChainNull final : public GpuSwapChain {
    public:
        void present() override {}
        void resizeBuffers(int width, int height) override {}
        rc<GpuTexture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class PipelineStateNull final : public GpuPipelineState {};

    class CommandListNull final : public GpuCommandList {
    public:
        void setPipelineState(GpuPipelineState* state) override {}

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override {}
        void clearDepthStencil(GpuResourceView* view) override {}

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override {}
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override {}

        void finish() override {}
        void clear(GpuPipelineState* pipelineState = nullptr) override {}

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override { return {}; }
        void unmap(GpuBuffer* resource, span<byte const> data) override {}
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override {}

        void bindRenderTarget(uint32 index, GpuResourceView* view) override {}
        void bindDepthStencil(GpuResourceView* view) override {}
        void bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset = 0) override {}
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override {}
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) override {}
        void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) override {}
        void bindSampler(uint32 slot, GpuSampler* sampler, GpuShaderStage stage) override {}
        void setPrimitiveTopology(GpuPrimitiveTopology topology) override {}
        void setViewport(GpuViewportDesc const& viewport) override {}
        void setClipRect(GpuClipRect rect) override {}
    };

    class BufferNull final : public GpuBuffer {
    public:
        BufferNull(GpuBufferType type) : _type(type) {}

        GpuBufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return 0; }

    private:
        GpuBufferType _type;
    };

    class TextureNull final : public GpuTexture {
    public:
        GpuTextureType type() const noexcept override { return GpuTextureType::Texture2D; }
        GpuFormat format() const noexcept override { return GpuFormat::Unknown; }
        glm::ivec3 dimensions() const noexcept override { return {1, 1, 0}; }
    };

    class SamplerNull final : public GpuSampler {};
} // namespace up::null
