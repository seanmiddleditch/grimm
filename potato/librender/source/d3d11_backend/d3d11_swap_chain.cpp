// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_swap_chain.h"
#include "d3d11_platform.h"
#include "d3d11_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"
#include "potato/spud/out_ptr.h"

#include <utility>

up::d3d11::SwapChainD3D11::SwapChainD3D11(com_ptr<IDXGISwapChain1> swapChain) : _swapChain(std::move(swapChain)) {}

up::d3d11::SwapChainD3D11::~SwapChainD3D11() = default;

auto up::d3d11::SwapChainD3D11::createSwapChain(IDXGIFactory2* factory, ID3D11Device* device, void* nativeWindow)
    -> rc<GpuSwapChain> {
    DXGI_SWAP_CHAIN_DESC1 desc = {0};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HWND window = static_cast<HWND>(nativeWindow);

    com_ptr<IDXGISwapChain1> swapChain;
    HRESULT hr = factory->CreateSwapChainForHwnd(device, window, &desc, nullptr, nullptr, out_ptr(swapChain));
    if (FAILED(hr) || swapChain == nullptr) {
        return nullptr;
    }

    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    return new_shared<SwapChainD3D11>(std::move(swapChain));
}

void up::d3d11::SwapChainD3D11::present() {
    _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    _bufferIndex = (_bufferIndex + 1) % 2;
}

void up::d3d11::SwapChainD3D11::resizeBuffers(int width, int height) {
    _swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
}

auto up::d3d11::SwapChainD3D11::getBuffer(int index) -> rc<GpuTexture> {
    com_ptr<ID3D11Resource> buffer;
    _swapChain->GetBuffer(index, __uuidof(ID3D11Resource), out_ptr(buffer));
    if (buffer == nullptr) {
        return nullptr;
    }
    return new_shared<TextureD3D11>(std::move(buffer));
}

int up::d3d11::SwapChainD3D11::getCurrentBufferIndex() {
    return _bufferIndex;
}
