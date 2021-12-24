// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_command_list.h"
#include "d3d11_imgui_renderer.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource_view.h"
#include "d3d11_sampler.h"
#include "d3d11_texture.h"

#include "potato/render/context.h"

#define BYTE unsigned char
#include "potato/shader/imgui_ps_5_0_shader.h"
#include "potato/shader/imgui_vs_5_0_shader.h"
#undef BYTE

#include <imgui.h>

namespace up::d3d11 {
    static constexpr up::uint32 bufferSize = 1024 * 1024;

    void ImguiRendererD3D11::beginFrame() {
        if (_sampler.empty()) {
            _initializeResources();
        }
    }

    void ImguiRendererD3D11::render(GpuCommandList& commandList) {
        ImGuiContext* const oldContext = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(&_context);

        ImGui::Render();
        ImDrawData& data = *ImGui::GetDrawData();

        ImGui::SetCurrentContext(oldContext);

        UP_GUARD_VOID(data.Valid, "ImguiBackend::draw() can only be called after Render() but before beginFrame()");

        UP_GUARD_VOID(data.TotalIdxCount * sizeof(ImDrawIdx) <= bufferSize, "Too many ImGui indices");
        UP_GUARD_VOID(data.TotalVtxCount * sizeof(ImDrawVert) <= bufferSize, "Too many ImGui verticies");

        commandList.setPipelineState(_pipelineState.get());
        commandList.setPrimitiveTopology(GpuPrimitiveTopology::Triangles);

        auto indices = commandList.map(_indexBuffer.get(), bufferSize);
        auto vertices = commandList.map(_vertexBuffer.get(), bufferSize);

        uint32 indexOffset = 0;
        uint32 vertexOffset = 0;

        for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
            auto const& list = *data.CmdLists[listIndex];

            std::memcpy(indices.data() + indexOffset, list.IdxBuffer.Data, list.IdxBuffer.Size * sizeof(ImDrawIdx));
            std::memcpy(vertices.data() + vertexOffset, list.VtxBuffer.Data, list.VtxBuffer.Size * sizeof(ImDrawVert));

            indexOffset += list.IdxBuffer.Size * sizeof(ImDrawIdx);
            vertexOffset += list.VtxBuffer.Size * sizeof(ImDrawVert);
        }

        commandList.unmap(_indexBuffer.get(), indices);
        commandList.unmap(_vertexBuffer.get(), vertices);

        float L = data.DisplayPos.x;
        float R = data.DisplayPos.x + data.DisplaySize.x;
        float T = data.DisplayPos.y;
        float B = data.DisplayPos.y + data.DisplaySize.y;
        float mvp[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };

        auto constants = commandList.map(_constantBuffer.get(), sizeof(mvp));
        std::memcpy(constants.data(), mvp, constants.size());
        commandList.unmap(_constantBuffer.get(), constants);

        commandList.bindIndexBuffer(_indexBuffer.get(), GpuIndexFormat::Unsigned16, 0);
        commandList.bindVertexBuffer(0, _vertexBuffer.get(), sizeof(ImDrawVert));
        commandList.bindConstantBuffer(0, _constantBuffer.get(), GpuShaderStage::Vertex);
        commandList.bindSampler(0, _sampler.get(), GpuShaderStage::Pixel);

        GpuViewportDesc viewport;
        viewport.width = data.DisplaySize.x;
        viewport.height = data.DisplaySize.y;
        viewport.leftX = 0;
        viewport.topY = 0;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
        commandList.setViewport(viewport);

        indexOffset = 0;
        vertexOffset = 0;

        ImVec2 pos = data.DisplayPos;

        for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
            auto const& list = *data.CmdLists[listIndex];

            for (int cmdIndex = 0; cmdIndex != list.CmdBuffer.Size; ++cmdIndex) {
                auto const& cmd = list.CmdBuffer.Data[cmdIndex];

                if (cmd.UserCallback != nullptr) {
                    cmd.UserCallback(&list, &cmd);
                    continue;
                }

                auto const srv = static_cast<GpuResourceView*>(cmd.TextureId);
                commandList.bindShaderResource(0, srv != nullptr ? srv : _srv.get(), GpuShaderStage::Pixel);

                GpuClipRect scissor;
                scissor.left = (uint32)cmd.ClipRect.x - (uint32)pos.x;
                scissor.top = (uint32)cmd.ClipRect.y - (uint32)pos.y;
                scissor.right = (uint32)cmd.ClipRect.z - (uint32)pos.x;
                scissor.bottom = (uint32)cmd.ClipRect.w - (uint32)pos.y;
                commandList.setClipRect(scissor);

                commandList.drawIndexed(cmd.ElemCount, indexOffset, vertexOffset);

                indexOffset += cmd.ElemCount;
            }

            vertexOffset += list.VtxBuffer.Size;
        }
    }

    void ImguiRendererD3D11::_initializeResources() {
        GpuInputLayoutElement inputLayout[] = {
            {GpuFormat::R32G32Float, GpuShaderSemantic::Position, 0, 0},
            {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
            {GpuFormat::R8G8B8A8UnsignedNormalized, GpuShaderSemantic::Color, 0, 0},
        };

        GpuPipelineStateDesc desc;
        desc.enableScissor = true;
        desc.vertShader = span{g_vertex_main}.as_bytes();
        desc.pixelShader = span{g_pixel_main}.as_bytes();
        desc.inputLayout = inputLayout;

        _indexBuffer = _device.createBuffer({.type = GpuBufferType::Index, .size = bufferSize}, {});
        _vertexBuffer = _device.createBuffer({.type = GpuBufferType::Vertex, .size = bufferSize}, {});
        _constantBuffer = _device.createBuffer({.type = GpuBufferType::Constant, .size = sizeof(float) * 16}, {});
        _pipelineState = _device.createPipelineState(desc);

        ImGuiContext* const oldContext = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(&_context);

        auto& imguiIO = ImGui::GetIO();

        int fontWidth = 0;
        int fontHeight = 0;
        unsigned char* pixels = nullptr;
        imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &fontWidth, &fontHeight);

        ImGui::SetCurrentContext(oldContext);

        GpuTextureDesc texDesc;
        texDesc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        texDesc.type = GpuTextureType::Texture2D;
        texDesc.width = fontWidth;
        texDesc.height = fontHeight;

        GpuDataDesc texData;
        texData.pitch = fontWidth * 4;
        texData.data = span{pixels, static_cast<uint32>(fontHeight * texData.pitch)}.as_bytes();

        auto font = _device.createTexture2D(texDesc, texData);
        _srv = _device.createShaderResourceView(font.get());

        _sampler = _device.createSampler();
    }
} // namespace up::d3d11
