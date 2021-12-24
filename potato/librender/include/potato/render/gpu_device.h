// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"

struct ImGuiContext;

namespace up {
    class GpuCommandList;
    class GpuPipelineLayout;
    class GpuPipelineState;
    class GpuResource;
    class GpuResourceView;
    class GpuSampler;
    class GpuSwapChain;
    class GpuTexture;
    class AssetLoader;
    class DebugDrawRenderer;
    class ImguiRenderer;

    struct GpuPipelineStateDesc;
    struct GpuTextureDesc;

    class GpuDevice : public shared<GpuDevice> {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual rc<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual rc<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual rc<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;
        virtual rc<GpuResource> createBuffer(GpuBufferType type, uint64 size) = 0;
        virtual rc<GpuResource> createTexture2D(GpuTextureDesc const& desc, GpuDataDesc const& data = {}) = 0;
        virtual rc<GpuSampler> createSampler() = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) = 0;
        virtual box<GpuResourceView> createDepthStencilView(GpuResource* depthStencilBuffer) = 0;
        virtual box<GpuResourceView> createShaderResourceView(GpuResource* resource) = 0;

        virtual void beginImguiFrame(ImGuiContext& context) = 0;
        virtual void renderImgui(ImGuiContext& context, GpuCommandList& commandList) = 0;
        virtual void renderDebugDraw(GpuCommandList& commandList) = 0;
    };
} // namespace up
