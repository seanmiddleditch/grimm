// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/rc.h"

#include <glm/fwd.hpp>

namespace up {
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
    class GpuResourceView;
    class GpuTexture;
} // namespace up

namespace up {
    class RenderContext;

    class RenderCamera {
    public:
        UP_RENDER_API RenderCamera();
        UP_RENDER_API ~RenderCamera();

        UP_RENDER_API void resetBackBuffer(rc<GpuTexture> texture);

        UP_RENDER_API void updateBuffers(
            RenderContext& ctx,
            glm::vec3 dimensions,
            glm::vec3 cameraPosition,
            glm::mat4x4 cameraTransform);
        UP_RENDER_API void beginFrame(RenderContext& ctx, glm::vec3 cameraPosition, glm::mat4x4 cameraTransform);

    private:
        box<GpuBuffer> _cameraDataBuffer;
        rc<GpuTexture> _backBuffer;
        rc<GpuTexture> _depthStencilBuffer;
        box<GpuResourceView> _rtv;
        box<GpuResourceView> _dsv;
    };
} // namespace up
