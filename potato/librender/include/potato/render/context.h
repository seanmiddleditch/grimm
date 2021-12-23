// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/platform.h"
#include "potato/spud/rc.h"

#include <glm/fwd.hpp>

namespace up {
    class GpuResource;
    class GpuCommandList;
    class GpuDevice;
    class GpuResourceView;
    class GpuResource;
    class Renderer;

    class RenderContext {
    public:
        UP_RENDER_API RenderContext(GpuDevice& device, rc<GpuCommandList> commandList);
        UP_RENDER_API ~RenderContext();

        UP_RENDER_API void bindBackBuffer(rc<GpuResource> target, rc<GpuResource> depthStencil = nullptr);
        UP_RENDER_API void UP_VECTORCALL applyCameraPerspective(glm::vec3 position, glm::vec3 forward, glm::vec3 up);
        UP_RENDER_API void applyCameraScreen();

        UP_RENDER_API void finish();

        GpuCommandList& commandList() noexcept { return *_commandList; }
        GpuDevice& device() noexcept { return _device; }

    private:
        void UP_VECTORCALL _applyCamera(glm::vec3 position, glm::mat4x4 cameraMatrix);

        GpuDevice& _device;
        rc<GpuCommandList> _commandList;
        rc<GpuResource> _cameraDataBuffer;
        rc<GpuResource> _backBuffer;
        rc<GpuResource> _depthStencilBuffer;
        box<GpuResourceView> _rtv;
        box<GpuResourceView> _dsv;
    };
} // namespace up
