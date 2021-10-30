// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/delegate_ref.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class GpuSwapChain;
    class GpuTexture;
    class AssetLoader;
    class GpuRenderable;

    struct GpuPipelineStateDesc;
    struct GpuTextureDesc;
    struct FrameData;

    class IRenderable;

    class GpuDevice : public shared<GpuDevice> {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual box<GpuRenderable> createRenderable(IRenderable* pInterface) = 0;
        virtual rc<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;
        virtual box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) = 0;
        virtual rc<GpuTexture> createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* spawpChain) = 0;
        virtual rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) = 0;
        virtual box<GpuSampler> createSampler() = 0;

        virtual box<GpuResourceView> createShaderResourceView(GpuPipelineState* pipeline, GpuTexture* resource) = 0;
        virtual box<GpuResourceView> createRenderTargetView(GpuTexture* resource) = 0;
        virtual box<GpuResourceView> createDepthStencilView(GpuTexture* resource) = 0;

        virtual void beginFrame(GpuSwapChain* swapChain) = 0;
        virtual void endFrame(GpuSwapChain* swapChain) = 0;

        // temp API for managing resource creation commands
        virtual void beginResourceCreation() = 0;
        virtual void endResourceCreation() = 0;

        virtual void render(const FrameData& frameData, GpuRenderable* renderable) = 0;
        virtual void execute(bool quitting) = 0;

        virtual void clearCommandList() = 0;

        virtual view<unsigned char> getDebugShader(GpuShaderStage stage) = 0;
        virtual void debugDraw(delegate_ref<void(GpuCommandList& cmdList)> callback) = 0; 

        virtual void registerAssetBackends(AssetLoader& assetLoader) = 0;
    };
} // namespace up
