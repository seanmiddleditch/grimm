// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "game_editor.h"

#include "potato/editor/editor.h"
#include "potato/editor/workspace.h"
#include "potato/editor/imgui_ext.h"
#include "potato/game/components/camera_component.h"
#include "potato/game/components/camera_controllers.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/transform.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/renderer.h"

#include <glm/glm.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        struct PlayPauseCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editors.game.commands.play_pause"},
                .displayName = "Play / Pause",
                .icon = ICON_FA_PLAY,
                .hotkey = "F5",
                .menu = "Actions\\Play/Pause"};
        };

        class GameEditorFactory : public EditorFactory<GameEditor> {
        public:
            explicit GameEditorFactory(AudioEngine& audio) : _audio(audio) { }

            box<EditorBase> createEditor(EditorParams const& params) override {
                return new_box<GameEditor>(params, _audio, nullptr);
            }

        private:
            AudioEngine& _audio;
        };
    } // namespace

    struct GameEditor::PlayPauseHandler : CommandHandler<PlayPauseCommand> {
        PlayPauseHandler(GameEditor& editor) : _editor(editor) { }

        void invoke(PlayPauseCommand&) override { _editor.togglePause(); }

    private:
        GameEditor& _editor;
    };

    void GameEditor::addFactory(Workspace& workspace, AudioEngine& audio) {
        workspace.addFactory<GameEditorFactory>(audio);
    }

    GameEditor::GameEditor(EditorParams const& params, AudioEngine& audio, box<Space> space)
        : Editor(params)
        , _space(std::move(space)) {
        commandScope().addHandler<PlayPauseHandler>(*this);

        Space::addDemoSystem(*_space, audio);

        _space->start();
    }

    void GameEditor::addCommands(CommandManager& commands) { commands.addCommand<PlayPauseCommand>(); }

    void GameEditor::content(CommandManager&) {
        auto const contentId = ImGui::GetID("GameContentView");
        auto const* const ctx = ImGui::GetCurrentContext();
        auto const& io = ImGui::GetIO();

        if (ImGui::BeginMenuBar()) {
            auto const icon = _paused ? ICON_FA_PLAY : ICON_FA_STOP;
            auto const text = _paused ? "Play" : "Pause";
            auto const xPos = ImGui::GetWindowSize().x * 0.5f - ImGui::CalcTextSize(text).x * 0.5f -
                ImGui::GetStyle().ItemInnerSpacing.x;
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
            // TODO: https://github.com/potatoengine/potato/issues/305
            SDL_SetRelativeMouseMode(SDL_TRUE);

            int mouseRelX = 0;
            int mouseRelY = 0;
            SDL_GetRelativeMouseState(&mouseRelX, &mouseRelY);

            glm::vec3 const relMotion = {
                static_cast<float>(mouseRelX) / io.DisplaySize.x,
                static_cast<float>(mouseRelY) / io.DisplaySize.y,
                static_cast<float>(io.MouseWheel > 0.f) - static_cast<float>(io.MouseWheel < 0.f)};

            glm::vec3 const relMove = {
                static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_D)) - static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_A)),
                static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_SPACE)) -
                    static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_C)),
                static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_W)) -
                    static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_S))};

            _space->entities().select<FlyCameraComponent>([&](EntityId, FlyCameraComponent& cam) {
                cam.relativeMovement = relMove;
                cam.relativeMotion = relMotion;
            });
        }
        else {
            // TODO: https://github.com/potatoengine/potato/issues/305
            SDL_SetRelativeMouseMode(SDL_FALSE);
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
                ImGui::Image(_bufferView->getImguiTexture(), contentSize);
            }
            ImGui::SetCursorPos(pos);
            ImGui::InvisibleButton("GameContent", contentSize);
            if (ImGui::IsItemActive() && _space != nullptr && !_paused) {
                _isInputBound = true;
            }
        }
        ImGui::EndChild();
    }

    void GameEditor::tick(float deltaTime) {
        if (!_paused) {
            _space->update(deltaTime);
        }
    }

    void GameEditor::render(Renderer& renderer, float deltaTime) {
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

            ctx.finish();
        }
    }

    void GameEditor::_resize(GpuDevice& device, glm::ivec2 size) {
        using namespace up;
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.bind = GpuBindFlags::RenderTarget | GpuBindFlags::ShaderResource;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = device.createTexture2D(desc, {});

        _bufferView = device.createShaderResourceView(_buffer.get());
    }
} // namespace up::shell
