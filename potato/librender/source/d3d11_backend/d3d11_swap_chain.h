// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_swap_chain.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d11 {
    class SwapChainD3D11 : public GpuSwapChain {
    public:
        SwapChainD3D11(com_ptr<IDXGISwapChain1> swapChain);
        virtual ~SwapChainD3D11();

        SwapChainD3D11(SwapChainD3D11&&) = delete;
        SwapChainD3D11& operator=(SwapChainD3D11&&) = delete;

        static rc<GpuSwapChain> createSwapChain(IDXGIFactory2* factory, ID3D11Device* device, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        rc<GpuTexture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        com_ptr<IDXGISwapChain1> _swapChain;
        int _bufferIndex = 0;
    };
} // namespace up::d3d11
