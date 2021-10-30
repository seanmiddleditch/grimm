// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_renderable.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up {
    class IRenderable;
    class RenderContext; 
};

namespace up::d3d12 {
    class ContextD3D12;

    class RenderableD3D12 final : public GpuRenderable {
    public:
        explicit RenderableD3D12(IRenderable* renderable);
        virtual ~RenderableD3D12();

        RenderableD3D12(RenderableD3D12&&) = delete;
        RenderableD3D12& operator=(RenderableD3D12&&) = delete;

        bool create(); 
        void onRender(RenderContext& ctx);

    private:
        IRenderable* _renderable;
    };
} // namespace up::d3d12
