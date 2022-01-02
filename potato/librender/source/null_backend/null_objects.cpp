// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "null_objects.h"

namespace up::null {

    void FactoryNull::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
        static GpuDeviceInfo deviceInfo = {0};

        callback(deviceInfo);
    }

    auto FactoryNull::createDevice(int index) -> rc<GpuDevice> { return new_shared<DeviceNull>(); }

    auto DeviceNull::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> { return new_shared<SwapChainNull>(); }

    auto DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> rc<GpuCommandList> {
        return new_shared<CommandListNull>();
    }

    auto DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> rc<GpuPipelineState> {
        return new_shared<PipelineStateNull>();
    }

    auto DeviceNull::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
        return new_box<ResourceViewNull>(GpuViewType::RTV);
    }

    auto DeviceNull::createDepthStencilView(GpuResource* depthStencilBuffer) -> box<GpuResourceView> {
        return new_box<ResourceViewNull>(GpuViewType::DSV);
    }

    auto DeviceNull::createShaderResourceView(GpuResource* resource) -> box<GpuResourceView> {
        return new_box<ResourceViewNull>(GpuViewType::SRV);
    }

    auto DeviceNull::createBuffer(GpuBufferDesc const& desc, GpuDataDesc const& data) -> rc<GpuResource> {
        return new_shared<BufferNull>(desc.type);
    }

    auto DeviceNull::createTexture2D(GpuTextureDesc const& desc, GpuDataDesc const& data) -> rc<GpuResource> {
        return new_shared<TextureNull>();
    }

    auto DeviceNull::createSampler(GpuSamplerDesc const& desc) -> rc<GpuSampler> { return new_shared<SamplerNull>(); }

    void DeviceNull::beginImguiFrame(ImGuiContext&) { }

    void DeviceNull::renderImgui(ImGuiContext&, GpuCommandList&) { }

    void DeviceNull::renderDebugDraw(GpuCommandList&) { }

    auto SwapChainNull::getBuffer(int index) -> rc<GpuResource> { return new_shared<TextureNull>(); }

    int SwapChainNull::getCurrentBufferIndex() { return 0; }
} // namespace up::null

auto up::CreateFactoryNull() -> box<GpuDeviceFactory> {
    return new_box<null::FactoryNull>();
}
