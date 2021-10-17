// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_pipeline_state.h"
#include "d3d12_command_list.h"
#include "d3d12_desc_heap.h"
#include "d3d12_root_signature.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

static D3D12_BLEND_DESC defaultBlendState() {
    D3D12_BLEND_DESC blend = {};
    blend.AlphaToCoverageEnable = FALSE;
    blend.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC rtBlendOps = {};

    rtBlendOps.BlendEnable = TRUE;
    rtBlendOps.LogicOpEnable = FALSE;
    rtBlendOps.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    rtBlendOps.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    rtBlendOps.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendOps.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    rtBlendOps.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendOps.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendOps.LogicOp = D3D12_LOGIC_OP_CLEAR;
    rtBlendOps.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (up::uint32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        blend.RenderTarget[i] = rtBlendOps;
    }

    return blend;
}

static D3D12_RASTERIZER_DESC defaultRestirizerState() {
    D3D12_RASTERIZER_DESC rs = {};

    rs.FillMode = D3D12_FILL_MODE_SOLID;
    rs.CullMode = D3D12_CULL_MODE_NONE;

    rs.FrontCounterClockwise = FALSE;
    rs.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rs.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rs.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rs.DepthClipEnable = TRUE;
    rs.MultisampleEnable = FALSE;
    rs.AntialiasedLineEnable = FALSE;
    rs.ForcedSampleCount = 0;
    rs.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    return rs;
}

static D3D12_DEPTH_STENCIL_DESC defeaultDepthStencilState() {
    D3D12_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp =
        D3D12_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.BackFace = desc.FrontFace;

    return desc;
}


up::d3d12::PipelineStateD3D12::PipelineStateD3D12()  {
}

up::d3d12::PipelineStateD3D12::~PipelineStateD3D12() {
};

auto up::d3d12::PipelineStateD3D12::createGraphicsPipelineState(ID3D12Device* device, GpuPipelineStateDesc const& desc)
    -> box<PipelineStateD3D12> {
    UP_ASSERT(device != nullptr);
    auto pso = new_box<PipelineStateD3D12>();
    pso->create(device, desc); 
    return pso;
}

bool up::d3d12::PipelineStateD3D12::create(ID3D12Device* device, GpuPipelineStateDesc const& desc) {
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    uint32 offset = 0;
    for (auto i : desc.inputLayout) {
        D3D12_INPUT_ELEMENT_DESC desc = {
            toNative(i.semantic).c_str(),
            0,
            toNative(i.format),
            0,
            offset,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0};

        offset += toByteSize(i.format);
        elements.push_back(desc);
    }

    _signature = RootSignatureD3D12::getRootSignature(RootSignatureType::ImGui);

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.NodeMask = 1;
    psoDesc.InputLayout = {elements.data(), static_cast<UINT>(elements.size())};
    psoDesc.pRootSignature = _signature->signature();
    psoDesc.VS = {desc.vertShader.data(), desc.vertShader.size()};
    psoDesc.PS = {desc.pixelShader.data(), desc.pixelShader.size()};
    psoDesc.RasterizerState = defaultRestirizerState();
    psoDesc.BlendState = defaultBlendState();
    psoDesc.DepthStencilState = defeaultDepthStencilState();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    device->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), out_ptr(_state));

    _srvHeap = new_box<DescriptorHeapD3D12>(
        device,
        1,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    return true;
}
    

void up::d3d12::PipelineStateD3D12::bindPipeline(ID3D12GraphicsCommandList* cmd) {
    UP_ASSERT(cmd!=nullptr);

    cmd->SetGraphicsRootSignature(_signature->signature());
    cmd->SetPipelineState(_state.get());

    // Setup blend factor
    const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
    cmd->OMSetBlendFactor(blend_factor);
    
    ID3D12DescriptorHeap* ppHeaps[] = {_srvHeap->heap(), _samplerHeap};
    cmd->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void up::d3d12::PipelineStateD3D12::bindTexture(
    ID3D12GraphicsCommandList* cmd,
    D3D12_GPU_DESCRIPTOR_HANDLE srv,
    D3D12_GPU_DESCRIPTOR_HANDLE sampler) {
    UP_ASSERT(cmd != nullptr);

    cmd->SetGraphicsRootDescriptorTable(RootParamType::TextureSRV, srv);
    cmd->SetGraphicsRootDescriptorTable(RootParamType::TextureSampler, sampler);
}

void up::d3d12::PipelineStateD3D12::bindConstBuffer(ID3D12GraphicsCommandList* cmd, D3D12_GPU_VIRTUAL_ADDRESS cbv) {
    UP_ASSERT(cmd != nullptr);
    cmd->SetGraphicsRootConstantBufferView(RootParamType::ConstantBuffer, cbv);
}

void up::d3d12::PipelineStateD3D12::bindConstValues(ID3D12GraphicsCommandList* cmd, uint32 size, float* values) {
    UP_ASSERT(cmd != nullptr);
    cmd->SetGraphicsRoot32BitConstants(RootParamType::ConstantBuffer, size, values, 0);
}
