// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_swap_chain.h"
#include "d3d12_desc_heap.h"
#include "d3d12_platform.h"
#include "d3d12_texture.h"
#include "d3d12_resource_view.h"
#include "d3d12_device.h"

#include "d3d12_command_list.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"
#include "potato/spud/out_ptr.h"

#include <utility>

auto up::d3d12::SwapChainD3D12::createSwapChain(
    IDXGIFactoryType* factory,
    ID3DDeviceType* device,
    ID3D12CommandQueue* queue,
    DescriptorHeapD3D12* descHeap,
    void* nativeWindow)
    -> rc<GpuSwapChain> {
    auto swapchain = new_shared<SwapChainD3D12>();
    swapchain->create(factory, device, queue, descHeap, nativeWindow);
    return swapchain;
}

auto up::d3d12::SwapChainD3D12::create(
    IDXGIFactoryType* factory,
    ID3D12Device* device,
    ID3D12CommandQueue* queue,
    DescriptorHeapD3D12* descHeap,
    void* nativeWindow) -> bool {

    DXGI_SWAP_CHAIN_DESC1 desc = {0};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = kNumFrames;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HWND window = static_cast<HWND>(nativeWindow);

    com_ptr<IDXGISwapChain1> swapChain;
    HRESULT hr = factory->CreateSwapChainForHwnd(queue, window, &desc, nullptr, nullptr, out_ptr(swapChain));
    if (FAILED(hr) || swapChain == nullptr) {
        return false;
    }
   _swapChain = swapChain.as<IDXGISwapChainType>();

    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    _descHeap = descHeap;

    initBackBufferTargets(device);

    return true;
}

static inline D3D12_RESOURCE_BARRIER InitTransitionBarrier(
    _In_ ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter,
    UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept {
    D3D12_RESOURCE_BARRIER result = {};
    result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    result.Flags = flags;
    result.Transition.pResource = pResource;
    result.Transition.StateBefore = stateBefore;
    result.Transition.StateAfter = stateAfter;
    result.Transition.Subresource = subresource;
    return result;
}

void up::d3d12::SwapChainD3D12::bind(GpuCommandList* cmd) {
    auto cl = static_cast<CommandListD3D12*>(cmd);

    // Indicate that the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER barrier = InitTransitionBarrier(
        _backBuffer[_bufferIndex].get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    cl->getResource()->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{_descHeap->get_cpu(_bufferIndex)};
    cl->getResource()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
};

void up::d3d12::SwapChainD3D12::unbind(GpuCommandList* cmd) {
    auto cl = static_cast<CommandListD3D12*>(cmd);

    // Indicate that the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER barrier = InitTransitionBarrier(
        _backBuffer[_bufferIndex].get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cl->getResource()->ResourceBarrier(1, &barrier);
}

void up::d3d12::SwapChainD3D12::present() {
    _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    _bufferIndex = _swapChain->GetCurrentBackBufferIndex();
}

void up::d3d12::SwapChainD3D12::resizeBuffers(GpuDevice& device, int width, int height) {

    for (uint32 i = 0; i < kNumFrames; i++) {
        _backBuffer[i].reset();
    }

    _swapChain->ResizeBuffers(kNumFrames, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

    initBackBufferTargets(static_cast<DeviceD3D12&>(device).getDevice());

    _bufferIndex = _swapChain->GetCurrentBackBufferIndex();
}

auto up::d3d12::SwapChainD3D12::getBuffer(int index) -> rc<GpuTexture> {
    ID3DResourcePtr buffer;
    _swapChain->GetBuffer(_bufferIndex, __uuidof(ID3D12Resource), out_ptr(buffer));
    if (buffer == nullptr) {
        return nullptr;
    }
    return new_shared<TextureD3D12>(std::move(buffer));
}

int up::d3d12::SwapChainD3D12::getCurrentBufferIndex() {
    return _bufferIndex;
}

void up::d3d12::SwapChainD3D12::initBackBufferTargets(ID3D12Device* device) {
    for (uint32 i = 0; i < kNumFrames; i++) {
        _swapChain->GetBuffer(i, __uuidof(ID3D12Resource), out_ptr(_backBuffer[i]));
        device->CreateRenderTargetView(_backBuffer[i].get(), nullptr, _descHeap->get_cpu(i));
    }

    _bufferIndex = _swapChain->GetCurrentBackBufferIndex();
}
