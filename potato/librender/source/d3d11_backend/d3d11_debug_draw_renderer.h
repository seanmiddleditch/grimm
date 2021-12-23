// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_buffer.h"
#include "d3d11_device.h"

#include "potato/render/debug_draw.h"
#include "potato/render/shader.h"

struct ImGuiContext;

namespace up::d3d11 {
    class DebugDrawRendererD3D11 final {
    public:
        DebugDrawRendererD3D11(DeviceD3D11& device) : _device(device) { }

        void render(GpuCommandList& commandList);

    private:
        DeviceD3D11& _device;
        rc<GpuPipelineState> _debugState;
        rc<GpuResource> _debugBuffer;
        rc<GpuResource> _indexBuffer;
        rc<GpuResource> _vertexBuffer;
        rc<GpuResource> _constantBuffer;
        rc<GpuPipelineState> _pipelineState;
        box<GpuResourceView> _srv;
        rc<GpuSampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
    };
} // namespace up::d3d11
