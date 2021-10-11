// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "null_objects.h"

void up::null::FactoryNull::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    static GpuDeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto up::null::FactoryNull::createDevice(int index) -> rc<GpuDevice> {
    return new_shared<DeviceNull>();
}

auto up::CreateFactoryNull() -> box<GpuDeviceFactory> {
    return new_box<null::FactoryNull>();
}

auto up::null::DeviceNull::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    return new_shared<SwapChainNull>();
}

auto up::null::DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return new_box<CommandListNull>();
}

auto up::null::DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return new_box<PipelineStateNull>();
}

auto up::null::DeviceNull::createRenderTargetView(GpuTexture* renderTarget) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::RTV);
}

auto up::null::DeviceNull::createDepthStencilView(GpuTexture* depthStencilBuffer) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::DSV);
}

auto up::null::DeviceNull::createShaderResourceView(GpuBuffer* buffer) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::SRV);
}

auto up::null::DeviceNull::createShaderResourceView(GpuTexture* texture) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::SRV);
}

auto up::null::DeviceNull::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    return new_box<BufferNull>(type);
}

auto up::null::DeviceNull::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    return new_shared<TextureNull>();
}

auto up::null::DeviceNull::createSampler() -> box<GpuSampler> {
    return new_box<SamplerNull>();
}

auto up::null::DeviceNull::getDebugShader(GpuShaderStage) -> up::view<unsigned char> {
    return {};
}

auto up::null::SwapChainNull::getBuffer(int index) -> rc<GpuTexture> {
    return new_shared<TextureNull>();
}

int up::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
