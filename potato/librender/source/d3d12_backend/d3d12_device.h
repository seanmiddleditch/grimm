// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"

#include "potato/render/gpu_device.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/unique_resource.h"

namespace up {
    class GpuRenderable;
};

namespace up::d3d12 {
    class CommandListD3D12;
    class DescriptorHeapD3D12;
    class RenderContextD3D12;

    class DeviceD3D12 final : public GpuDevice {
    public:
        DeviceD3D12(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter);
        virtual ~DeviceD3D12();

        DeviceD3D12(DeviceD3D12&&) = delete;
        DeviceD3D12& operator=(DeviceD3D12&&) = delete;

        static rc<GpuDevice> createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter);

        box<GpuRenderable> createRenderable(IRenderable* pInterface) override;
        rc<GpuSwapChain> createSwapChain(void* nativeWindow) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) override;
        rc<GpuTexture> createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* swapChain) override;
        rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        box<GpuResourceView> createShaderResourceView(GpuPipelineState* pipeline, GpuTexture* resource) override;
        box<GpuResourceView> createRenderTargetView(GpuTexture* resource) override;
        box<GpuResourceView> createDepthStencilView(GpuTexture* resource) override;

        bool create();
        void render(const FrameData& frameData, GpuRenderable* renderable) override;
        void execute(bool quitting);

        void beginFrame(GpuSwapChain* swapChain) override;
        void endFrame(GpuSwapChain* swapChain) override;

        void beginResourceCreation() override;
        void endResourceCreation() override;

        void clearCommandList() override;

        view<unsigned char> getDebugShader(GpuShaderStage stage) override;
        void debugDraw(delegate_ref<void(GpuCommandList& cmdList)> callback) override;

        void registerAssetBackends(AssetLoader& assetLoader) override;

        ID3D12Device* getDevice() const { return _device.get(); }

    protected:
        void createDefaultSampler();
        void createAllocator();
        void createFrameSync();
        void waitForFrame();

    private:
        IDXGIFactoryPtr _factory;
        IDXGIAdapterPtr _adapter;
        ID3DDevicePtr _device;

        box<DescriptorHeapD3D12> _rtvHeap;
        box<DescriptorHeapD3D12> _dsvHeap;
        box<DescriptorHeapD3D12> _samplerHeap;

        ID3DCommandQueuePtr _commandQueue;

        // main command list -- for now we use single command list but that will most likely change
        // in the future
        box<CommandListD3D12> _mainCmdList;
        box<CommandListD3D12> _uploadCmdList;
        box<RenderContextD3D12> _mainContext;

        // Synchronization objects.
        UINT _frameIndex;
        HANDLE _fenceEvent;
        com_ptr<ID3D12Fence> _fence;
        UINT64 _fenceValue;

        // allocator
        com_ptr<D3D12MA::Allocator> _allocator;
    };
} // namespace up::d3d12
