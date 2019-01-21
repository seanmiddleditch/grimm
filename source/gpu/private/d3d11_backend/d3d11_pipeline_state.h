// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/pipeline_state.h"

namespace gm {
    struct PipelineStateParamsD3D11 {
        com_ptr<ID3D11RasterizerState> rasterState;
        com_ptr<ID3D11DepthStencilState> depthStencilState;
        com_ptr<ID3D11BlendState> blendState;
        com_ptr<ID3D11InputLayout> inputLayout;
        com_ptr<ID3D11VertexShader> vertShader;
    };

    class PipelineStateD3D11 : public GpuPipelineState {
    public:
        explicit PipelineStateD3D11(PipelineStateParamsD3D11 params);
        virtual ~PipelineStateD3D11();

        static box<PipelineStateD3D11> createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D11Device* device);

        com_ptr<ID3D11RasterizerState> const& rasterState() const noexcept { return _params.rasterState; }
        com_ptr<ID3D11DepthStencilState> const& depthStencilState() const noexcept { return _params.depthStencilState; }
        com_ptr<ID3D11BlendState> const& blendState() const noexcept { return _params.blendState; }
        com_ptr<ID3D11InputLayout> const& inputLayout() const noexcept { return _params.inputLayout; }
        com_ptr<ID3D11VertexShader> const& vertexShader() const noexcept { return _params.vertShader; }

    private:
        PipelineStateParamsD3D11 _params;
    };
} // namespace gm
