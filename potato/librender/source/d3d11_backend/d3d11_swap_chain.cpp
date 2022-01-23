// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_swap_chain.h"
#include "d3d11_platform.h"
#include "d3d11_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"
#include "potato/spud/out_ptr.h"

#include <utility>

namespace up::d3d11 {

    SwapChainD3D11::SwapChainD3D11(com_ptr<IDXGISwapChain1> swapChain) : _swapChain(std::move(swapChain)) { }

    SwapChainD3D11::~SwapChainD3D11() = default;

    auto SwapChainD3D11::createSwapChain(IDXGIFactory2* factory, ID3D11Device* device, void* nativeWindow)
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

    void SwapChainD3D11::present() { _swapChain->Present(0, DXGI_SWAP_EFFECT_FLIP_DISCARD); }

    void SwapChainD3D11::resizeBuffers(int width, int height) {
        _swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
    }

    auto SwapChainD3D11::getBuffer() -> rc<GpuResource> {
        com_ptr<ID3D11Resource> buffer;
        _swapChain->GetBuffer(0, __uuidof(ID3D11Resource), out_ptr(buffer));
        if (buffer == nullptr) {
            return nullptr;
        }
        return new_shared<TextureD3D11>(std::move(buffer));
    }

    void SwapChainD3D11::bindToContext(ID3D11Device* device, ID3D11DeviceContext* context) const {
        UP_GUARD_VOID(device != nullptr);
        UP_GUARD_VOID(context != nullptr);

        com_ptr<ID3D11Resource> buffer;
        _swapChain->GetBuffer(0, __uuidof(ID3D11Resource), out_ptr(buffer));
        UP_GUARD_VOID(buffer != nullptr);

        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        com_ptr<ID3D11RenderTargetView> view;
        [[maybe_unused]] HRESULT hr = device->CreateRenderTargetView(buffer.get(), &desc, out_ptr(view));
        UP_GUARD_VOID(SUCCEEDED(hr));

        ID3D11RenderTargetView* const rtv = view.get();
        context->OMSetRenderTargets(1, &rtv, nullptr);

        FLOAT rgba[4] = {};
        context->ClearRenderTargetView(view.get(), rgba);
    }
} // namespace up::d3d11
