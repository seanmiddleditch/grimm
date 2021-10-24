// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/concurrent_queue.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/vector.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuDevice;
    class RenderContext;
    class AssetLoader;
    class Material;
    class Mesh;
    class Shader;
    class Texture;
    class ResourceLoader;
    class GpuRenderable;
    class GpuSwapChain;
    class GpuTexture; 

    // client-side interface of render object.  
    class IRenderable {
    public: 
        virtual void onSchedule(RenderContext& context) = 0;
        virtual void onRender(RenderContext& context) = 0; 
    };

    class Renderer {
    public:
        UP_RENDER_API Renderer(rc<GpuDevice> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void flush();
        UP_RENDER_API void flushDebugDraw(float frameTime);

        // \brief: call before application is about to quit to ensure that the renderer can clean up
        //          any outstanding operations
        UP_RENDER_API void quit();

        UP_RENDER_API GpuRenderable* createRendarable(IRenderable* pInterface);

        UP_RENDER_API void registerAssetBackends(AssetLoader& assetLoader);
        UP_RENDER_API bool createSwapChain(void* nativeWindow);

        UP_RENDER_API void clearCommandList();

        UP_RENDER_API void resizeBuffers(int width, int height);
        UP_RENDER_API rc<GpuTexture> getBackBuffer();

        GpuDevice& device() const noexcept { return *_device; }
    
    private:
        rc<GpuSwapChain> _swapChain;
        rc<GpuDevice> _device;
        box<GpuBuffer> _frameDataBuffer;
        box<GpuPipelineState> _debugState;
        box<GpuBuffer> _debugBuffer;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
        vector<box<GpuRenderable>> _rendarables;
    };
} // namespace up
