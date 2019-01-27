// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/gpu/swap_chain.h"

namespace gm::gpu {
    class CommandList;
    class Device;
    class ResourceView;
    class Texture;
} // namespace gm::gpu

namespace gm {
    class Camera {
    public:
        GM_RENDER_API explicit Camera(rc<gpu::SwapChain> swapChain = nullptr);
        GM_RENDER_API ~Camera();

        GM_RENDER_API void resetSwapChain(rc<gpu::SwapChain> swapChain);

        GM_RENDER_API void beginFrame(gpu::CommandList& commandList, gpu::Device& device);
        GM_RENDER_API void endFrame(gpu::CommandList& commandList, gpu::Device& device);

    private:
        rc<gpu::SwapChain> _swapChain;
        box<gpu::Texture> _backBuffer;
        box<gpu::ResourceView> _rtv;
    };
} // namespace gm
