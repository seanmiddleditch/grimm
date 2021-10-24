// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"
#include "potato/spud/rc.h"

namespace up {
    class GpuCommandList;
    class GpuResourceView;
    class GpuDevice; 
    class GpuTexture;

    class GpuSwapChain : public shared<GpuSwapChain> {
    public:
        GpuSwapChain() = default;
        virtual ~GpuSwapChain() = default;

        GpuSwapChain(GpuSwapChain&&) = delete;
        GpuSwapChain& operator=(GpuSwapChain&&) = delete;

        virtual void bind(GpuCommandList* cmd) = 0;
        virtual void unbind(GpuCommandList* cmd) = 0;

        virtual void present() = 0;
        virtual void resizeBuffers(GpuDevice& device, int width, int height) = 0;
       // virtual box<GpuResourceView> getRenderTargetView() = 0;
        virtual rc<GpuTexture> getBuffer(int index) = 0;
        virtual int getCurrentBufferIndex() = 0;

    };
} // namespace up
