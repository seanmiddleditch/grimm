// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_command_list.h"
#include "d3d11_debug_draw_renderer.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource_view.h"
#include "d3d11_sampler.h"
#include "d3d11_texture.h"

#include "potato/render/context.h"

#define BYTE unsigned char
#include "potato/shader/debug_ps_5_0_shader.h"
#include "potato/shader/debug_vs_5_0_shader.h"
#undef BYTE

namespace up::d3d11 {
    static constexpr uint32 bufferSize = 64 * 1024;
    static constexpr uint32 maxVertsPerChunk = bufferSize / sizeof(DebugDrawVertex);

    void DebugDrawRendererD3D11::render(GpuCommandList& commandList) {
        if (_debugState.empty()) {
            // Create the debug pipeline
            GpuPipelineLayoutDesc pipeLayoutDesc;
            auto pipeLayout = _device.createPipelineLayout(pipeLayoutDesc);

            GpuInputLayoutElement const inputLayout[] = {
                {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
                {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
                {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
                {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
                {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
            };

            GpuPipelineStateDesc pipelineDesc;
            pipelineDesc.layout = pipeLayout.get();
            pipelineDesc.enableDepthTest = true;
            pipelineDesc.enableDepthWrite = true;
            pipelineDesc.vertShader = span{g_vertex_main}.as_bytes();
            pipelineDesc.pixelShader = span{g_pixel_main}.as_bytes();
            pipelineDesc.inputLayout = inputLayout;

            // Check to support null renderer; should this be explicit?
            if (!pipelineDesc.vertShader.empty() && !pipelineDesc.pixelShader.empty()) {
                _debugState = _device.createPipelineState(pipelineDesc);
            }
        }

        if (_debugBuffer == nullptr) {
            _debugBuffer = _device.createBuffer(GpuBufferType::Vertex, bufferSize);
        }

        commandList.setPipelineState(_debugState.get());
        commandList.bindVertexBuffer(0, _debugBuffer.get(), sizeof(DebugDrawVertex));
        commandList.setPrimitiveTopology(GpuPrimitiveTopology::Lines);

        dumpDebugDraw([this, &commandList](auto debugVertices) {
            if (debugVertices.empty()) {
                return;
            }

            uint32 vertCount = min(static_cast<uint32>(debugVertices.size()), maxVertsPerChunk);
            uint32 offset = 0;
            while (offset < debugVertices.size()) {
                commandList.update(_debugBuffer.get(), debugVertices.subspan(offset, vertCount).as_bytes());
                commandList.draw(vertCount);

                offset += vertCount;
                vertCount = min(static_cast<uint32>(debugVertices.size()) - offset, maxVertsPerChunk);
            }
        });
    }
} // namespace up::d3d11
