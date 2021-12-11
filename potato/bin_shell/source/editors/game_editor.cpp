// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "game_editor.h"

#include "potato/editor/imgui_ext.h"
#include "potato/game/components/camera_component.h"
#include "potato/game/components/camera_controllers.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/transform.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/renderer.h"
#include "potato/shell/editor.h"

#include <glm/glm.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

auto up::shell::createGameEditor(box<Space> space) -> box<Editor> {
    return new_box<GameEditor>(std::move(space));
}

void up::shell::GameEditor::configure() {
    addAction({.command = "Play / Pause", .menu = "Actions\\Play/Pause", .hotKey = "F5", .action = [this] {
                   _paused = !_paused;
               }});
}

void up::shell::GameEditor::content() {
    auto const contentId = ImGui::GetID("GameContentView");
    auto const* const ctx = ImGui::GetCurrentContext();
    auto const& io = ImGui::GetIO();

    if (ImGui::BeginMenuBar()) {
        auto const icon = _paused ? ICON_FA_PLAY : ICON_FA_STOP;
        auto const text = _paused ? "Play" : "Pause";
        auto const xPos =
            ImGui::GetWindowSize().x * 0.5f - ImGui::CalcTextSize(text).x * 0.5f - ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::SetCursorPosX(xPos);
        if (ImGui::MenuItemEx(text, icon, "F5")) {
            _paused = !_paused;
        }
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "Shift-ESC to release input");
        ImGui::EndMenuBar();
    }

    if (ImGui::IsKeyPressed(SDL_SCANCODE_F5, false)) {
        _paused = !_paused;
    }

    if (ImGui::IsKeyPressed(SDL_SCANCODE_ESCAPE) && io.KeyShift) {
        _isInputBound = false;
    }

    _isInputBound = _isInputBound && !_paused;

    if (_isInputBound) {
        ImGui::SetActiveID(contentId, ctx->CurrentWindow);
        ImGui::SetCaptureRelativeMouseMode(true);

        glm::vec3 relMotion = {0, 0, 0};
        glm::vec3 relMove = {0, 0, 0};

        relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
        relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
        relMotion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;

        relMove = {
            static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_D)) - static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_A)),
            static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_SPACE)) - static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_C)),
            static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_W)) - static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_S))};

        _space->entities().select<FlyCameraComponent>([&](EntityId, FlyCameraComponent& cam) {
            cam.relativeMovement = relMove;
            cam.relativeMotion = relMotion;
        });
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

    if (ImGui::BeginChild("GameContent", contentSize, false, ImGuiWindowFlags_NoScrollWithMouse)) {
        _viewDimensions = {contentSize.x, contentSize.y};

        auto const pos = ImGui::GetCursorScreenPos();
        if (_bufferView != nullptr) {
            ImGui::Image(_bufferView.get(), contentSize);
        }
        ImGui::SetCursorPos(pos);
        ImGui::InvisibleButton("GameContent", contentSize);
        if (ImGui::IsItemActive() && _space != nullptr && !_paused) {
            _isInputBound = true;
        }
    }
    ImGui::EndChild();
}

void up::shell::GameEditor::tick(float deltaTime) {
    _space->update(deltaTime);
}

void up::shell::GameEditor::render(Renderer& renderer, float deltaTime) {
    if (_viewDimensions.x == 0 || _viewDimensions.y == 0) {
        return;
    }

    glm::ivec2 bufferSize = _buffer != nullptr ? _buffer->dimensions() : glm::vec2{0, 0};
    if (bufferSize.x != _viewDimensions.x || bufferSize.y != _viewDimensions.y) {
        _resize(renderer.device(), _viewDimensions);
    }

    if (_cameraId == EntityId::None) {
        _cameraId = _space->entities().createEntity();
        _space->entities().addComponent<CameraComponent>(_cameraId);
        _space->entities().addComponent<FlyCameraComponent>(_cameraId);
        auto& trans = _space->entities().addComponent<TransformComponent>(_cameraId);

        Transform t;
        t.position = {0, 10, 15};
        t.lookAt({0, 0, 0});

        trans.position = t.position;
        trans.rotation = t.rotation;
    }

    if (_buffer != nullptr) {
        renderer.beginFrame();
        auto ctx = renderer.context();

        ctx.bindBackBuffer(_buffer);
        if (_space != nullptr) {
            _space->render(ctx);
        }
        renderer.endFrame(deltaTime);
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

    _bufferView = device.createShaderResourceView(_buffer.get());
}
