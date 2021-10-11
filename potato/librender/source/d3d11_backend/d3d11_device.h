// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_device.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/unique_resource.h"

namespace up::d3d11 {
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
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) override;
        rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        void execute(GpuCommandList* commandList) override;

        box<GpuResourceView> createRenderTargetView(GpuTexture* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuTexture* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) override;
        box<GpuResourceView> createShaderResourceView(GpuTexture* texture) override;

        view<unsigned char> getDebugShader(GpuShaderStage stage) override;

        void registerAssetBackends(AssetLoader& assetLoader) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D11Device> _device;
        com_ptr<ID3D11DeviceContext> _context;
    };
} // namespace up::d3d11
