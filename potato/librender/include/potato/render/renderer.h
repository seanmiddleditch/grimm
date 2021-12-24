// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/concurrent_queue.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class GpuResource;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuDevice;
    class RenderContext;
    class AssetLoader;

    class Renderer {
    public:
        UP_RENDER_API Renderer(rc<GpuDevice> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void beginFrame();

        UP_RENDER_API RenderContext context();
        GpuDevice& device() const noexcept { return *_device; }
        float frameTimestep() const noexcept { return static_cast<float>(_frameTimestep); }

        UP_RENDER_API rc<GpuCommandList> createCommandList() const noexcept;
        UP_RENDER_API void registerAssetBackends(AssetLoader& assetLoader);

        UP_RENDER_API void renderDebugDraw(GpuCommandList& commandList);

    private:
        rc<GpuDevice> _device;
        rc<GpuResource> _frameDataBuffer;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
        double _frameTimestep = 0;
    };
} // namespace up
