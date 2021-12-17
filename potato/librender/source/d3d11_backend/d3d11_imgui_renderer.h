// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_buffer.h"
#include "d3d11_device.h"

#include "potato/render/shader.h"

struct ImGuiContext;

namespace up::d3d11 {
    class ImguiRendererD3D11 final {
    public:
        ImguiRendererD3D11(ImGuiContext& context, DeviceD3D11& device) : _context(context), _device(device) { }

        void beginFrame();
        void render(GpuCommandList& commandList);

    private:
        void _initializeResources();

        ImGuiContext& _context;
        DeviceD3D11& _device;
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
