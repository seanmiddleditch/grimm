// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "game_editor.h"
#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"

#include "potato/editor/imgui_ext.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/renderer.h"

#include <glm/glm.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

auto up::shell::createGameEditor(rc<Scene> scene) -> box<Editor> {
    return new_box<GameEditor>(std::move(scene));
}

void up::shell::GameEditor::configure() {
    addAction({.command = "Play / Pause", .menu = "Actions\\Play/Pause", .hotKey = "F5", .action = [this] {
                   _wantPlaying = !_wantPlaying;
               }});
}

void up::shell::GameEditor::content() {
    auto const contentId = ImGui::GetID("GameContentView");
    auto const* const ctx = ImGui::GetCurrentContext();
    auto const& io = ImGui::GetIO();

    if (ImGui::BeginMenuBar()) {
        auto const icon = _scene->playing() ? ICON_FA_STOP : ICON_FA_PLAY;
        auto const text = _scene->playing() ? "Pause" : "Play";
        auto const xPos =
            ImGui::GetWindowSize().x * 0.5f - ImGui::CalcTextSize(text).x * 0.5f - ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::SetCursorPosX(xPos);
        if (ImGui::IconMenuItem(text, icon, "F5")) {
            _wantPlaying = !_wantPlaying;
        }
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "Shift-ESC to release input");
        ImGui::EndMenuBar();
    }

    if (ImGui::IsKeyPressed(SDL_SCANCODE_F5, false)) {
        _wantPlaying = !_wantPlaying;
    }

    if (_wantPlaying != _scene->playing()) {
        _scene->playing(_wantPlaying);
    }

    if (ImGui::IsKeyPressed(SDL_SCANCODE_ESCAPE) && io.KeyShift) {
        _isInputBound = false;
    }

    _isInputBound = _isInputBound && _scene->playing();

    if (_isInputBound) {
        ImGui::SetActiveID(contentId, ctx->CurrentWindow);
        ImGui::SetCaptureRelativeMouseMode(true);

        glm::vec3 relMotion = {0, 0, 0};
        glm::vec3 relMove = {0, 0, 0};

        relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
        relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
        relMotion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;

        relMove = {
            static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_D)) -
                static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_A)),
            static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_SPACE)) -
                static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_C)),
            static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_W)) -
                static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_S))};

        _cameraController.apply(_camera, relMove, relMotion, io.DeltaTime);
    }
    else {
        if (ctx->ActiveId == contentId) {
            ImGui::ClearActiveID();
        }
    }

    auto const contentSize = ImGui::GetContentRegionAvail();

    if (contentSize.x <= 0 || contentSize.y <= 0) {
        return;
    }

    if (ImGui::BeginChild("GameContent", contentSize, false)) {
        _sceneDimensions = {contentSize.x, contentSize.y};

        auto const pos = ImGui::GetCursorScreenPos();
        if (_bufferView != nullptr) {
            ImGui::Image(_bufferView.get(), contentSize);
        }
        ImGui::SetCursorPos(pos);
        ImGui::InvisibleButton("GameContent", contentSize);
        if (ImGui::IsItemActive() && _scene != nullptr && _scene->playing()) {
            _isInputBound = true;
        }
    }
    ImGui::EndChild();
}

void up::shell::GameEditor::render(Renderer& renderer, float deltaTime) {
    if (_sceneDimensions.x == 0 || _sceneDimensions.y == 0) {
        return;
    }

    glm::ivec2 bufferSize = _buffer != nullptr ? _buffer->dimensions() : glm::vec2{0, 0};
    if (bufferSize.x != _sceneDimensions.x || bufferSize.y != _sceneDimensions.y) {
        _resize(renderer.device(), _sceneDimensions);
    }

    if (_renderCamera == nullptr) {
        _renderCamera = new_box<RenderCamera>();
    }

    if (_buffer != nullptr && _scene != nullptr) {
        _scene->render(renderer);
    }
}

void up::shell::GameEditor::_resize(GpuDevice& device, glm::ivec2 size) {
    using namespace up;
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _buffer = device.createTexture2D(desc, {});

    //_bufferView = device.createShaderResourceView(_buffer.get());
}
