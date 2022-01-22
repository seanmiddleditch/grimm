// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"

#include "potato/render/gpu_device.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/unique_resource.h"

namespace up::d3d11 {
    class DebugDrawRendererD3D11;

    class DeviceD3D11 final : public GpuDevice {
    public:
        DeviceD3D11(
            com_ptr<IDXGIFactory2> factory,
            com_ptr<IDXGIAdapter1> adapter,
            com_ptr<ID3D11Device> device,
            com_ptr<ID3D11DeviceContext> context);
        virtual ~DeviceD3D11();

        DeviceD3D11(DeviceD3D11&&) = delete;
        DeviceD3D11& operator=(DeviceD3D11&&) = delete;

        static rc<GpuDevice> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        rc<GpuSwapChain> createSwapChain(void* nativeWindow) override;
        rc<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        rc<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        rc<GpuResource> createBuffer(GpuBufferDesc const& desc, GpuDataDesc const& data) override;
        rc<GpuResource> createTexture2D(GpuTextureDesc const& desc, GpuDataDesc const& data) override;
        rc<GpuSampler> createSampler(GpuSamplerDesc const& desc) override;

        void execute(GpuCommandList* commandList) override;

        box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuResource* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuResource* resource) override;

        void initImgui(ImGuiContext& context) override;
        void beginImguiFrame(ImGuiContext& context) override;
        void renderImgui(ImGuiContext& context, GpuSwapChain& swapChain) override;
        void renderDebugDraw(GpuCommandList& commandList) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D11Device> _device;
        com_ptr<ID3D11DeviceContext> _context;
        box<DebugDrawRendererD3D11> _debugDrawer;
        bool _imguiInitialized = false;
    };
} // namespace up::d3d11
