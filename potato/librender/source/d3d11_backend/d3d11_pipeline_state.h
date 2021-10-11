// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_pipeline_state.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d11 {
    struct PipelineStateParamsD3D11 {
        com_ptr<ID3D11RasterizerState> rasterState;
        com_ptr<ID3D11DepthStencilState> depthStencilState;
        com_ptr<ID3D11BlendState> blendState;
        com_ptr<ID3D11InputLayout> inputLayout;
        com_ptr<ID3D11VertexShader> vertShader;
        com_ptr<ID3D11PixelShader> pixelShader;
    };

    class PipelineStateD3D11 : public GpuPipelineState {
    public:
        explicit PipelineStateD3D11(PipelineStateParamsD3D11 params);
        virtual ~PipelineStateD3D11();

        static box<PipelineStateD3D11> createGraphicsPipelineState(
            GpuPipelineStateDesc const& desc,
            ID3D11Device* device);

        PipelineStateParamsD3D11 const& params() const noexcept { return _params; }

    private:
        static constexpr uint32 maxInputLayoutElements = 32;

        PipelineStateParamsD3D11 _params;
    };
} // namespace up::d3d11
