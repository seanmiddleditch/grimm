// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"

#include "potato/render/gpu_swap_chain.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class DescriptorHeapD3D12;

    class SwapChainD3D12 : public GpuSwapChain {
    public:
        SwapChainD3D12() = default;
        virtual ~SwapChainD3D12() = default;

        SwapChainD3D12(SwapChainD3D12&&) = delete;
        SwapChainD3D12& operator=(SwapChainD3D12&&) = delete;

        static rc<GpuSwapChain> createSwapChain(
            IDXGIFactoryType* factory,
            ID3D12Device* device,
            ID3D12CommandQueue* queue,
            DescriptorHeapD3D12* descHeap,
            void* nativeWindow);

        bool create(
            IDXGIFactoryType* factory,
            ID3D12Device* device,
            ID3D12CommandQueue* queue,
            DescriptorHeapD3D12* descHeap,
            void* nativeWindow);
        void bind(GpuCommandList* cmd) override;
        void unbind(GpuCommandList* cmd) override;
        void present() override;
        void resizeBuffers(GpuDevice& device, int width, int height) override;
        rc<GpuTexture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    protected:
        void initBackBufferTargets(ID3D12Device* device);

    private:
        static const up::uint32 kNumFrames = 2;

        IDXGISwapChainPtr _swapChain;
        uint32 _bufferIndex = 0;
        ID3DResourcePtr _backBuffer[kNumFrames];
        DescriptorHeapD3D12* _descHeap = nullptr;
    };
} // namespace up::d3d12
