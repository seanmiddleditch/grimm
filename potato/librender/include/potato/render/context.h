// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/platform.h"
#include "potato/spud/rc.h"

#include <glm/fwd.hpp>

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class GpuResourceView;
    class GpuTexture;

    class RenderContext {
    public:
        UP_RENDER_API RenderContext(GpuDevice& device, GpuCommandList& commands, double frameTime);
        UP_RENDER_API ~RenderContext();

        UP_RENDER_API void bindBackBuffer(rc<GpuTexture> target, rc<GpuTexture> depthStencil = nullptr);
        UP_RENDER_API void UP_VECTORCALL applyCameraPerspective(glm::vec3 position, glm::vec3 forward, glm::vec3 up);
        UP_RENDER_API void applyCameraScreen();

        GpuCommandList& commandList() noexcept { return _commandList; }
        GpuDevice& device() noexcept { return _device; }

    private:
        void UP_VECTORCALL _applyCamera(glm::vec3 position, glm::mat4x4 cameraMatrix);

        double _frameTime = 0;
        GpuCommandList& _commandList;
        GpuDevice& _device;
        box<GpuBuffer> _cameraDataBuffer;
        rc<GpuTexture> _backBuffer;
        rc<GpuTexture> _depthStencilBuffer;
        box<GpuResourceView> _rtv;
        box<GpuResourceView> _dsv;
    };
} // namespace up
