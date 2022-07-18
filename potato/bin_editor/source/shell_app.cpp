// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "shell_app.h"
#include "commands.h"
#include "edit_components.h"
#include "editors/asset_editor.h"
#include "editors/game_editor.h"
#include "editors/log_editor.h"
#include "editors/material_editor.h"
#include "editors/scene_editor.h"

#include "potato/audio/sound_resource.h"
#include "potato/editor/desktop.h"
#include "potato/editor/imgui_ext.h"
#include "potato/editor/imgui_fonts.h"
#include "potato/editor/project.h"
#include "potato/game/space.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/material.h"
#include "potato/render/renderer.h"
#include "potato/render/shader.h"
#include "potato/schema/recon_messages_schema.h"
#include "potato/schema/scene_schema.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/platform.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

#include <nlohmann/json.hpp>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <Tracy.hpp>
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>

// SDL includes X.h on Linux, which pollutes this name we care about
// FIXME: clean up this file for better abstractions to avoid header pollution problems
#if defined(Success)
#    undef Success
#endif

namespace up {
    namespace {
        struct QuitCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.quit"},
                .displayName = "Quit",
                .icon = ICON_FA_DOOR_OPEN,
                .hotkey = "Alt+F4",
                .menu = "File\\Quit::z_quit"};
        };

        struct OpenProjectCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.project.open"},
                .displayName = "Open Project",
                .icon = ICON_FA_FOLDER_OPEN,
                .hotkey = "Alt+Shift+O",
                .menu = "File\\Open Project::3_project::100"};
        };

        struct CloseProjectCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.project.close"},
                .displayName = "Close Project",
                .menu = "File\\Close Project::3_project::1100"};
        };

        struct ShowLogsCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editor.logs"},
                .displayName = "Show Logs",
                .icon = ICON_FA_FOLDER_OPEN,
                .hotkey = "Alt+Shift+L",
                .menu = "View\\Logs::3_project::100"};
        };

        struct ImportResourcesCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.project.import"},
                .displayName = "Import Resources",
                .icon = ICON_FA_FILE_IMPORT,
                .menu = "File\\Import Resources::3_project::120"};
        };

        struct ShowAboutCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editor.about"},
                .displayName = "Show About",
                .icon = ICON_FA_QUESTION_CIRCLE,
                .menu = "Help\\About"};
        };

        struct ShowImguiDemoCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editor.imgui_demo"},
                .displayName = "Imgui Demo",
                .menu = "Help\\Imgui Demo"};
        };

        struct CommandPaletteCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editor.command-palette.open"},
                .displayName = "Commands",
                .icon = ICON_FA_TERMINAL,
                .hotkey = "Ctrl+Shift+P",
                .menu = "Edit\\Commands"};
        };

        struct QuitHandler final : CommandHandler<QuitCommand> {
            QuitHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(QuitCommand&) override { _app.quit(); }

        private:
            shell::ShellApp& _app;
        };

        struct OpenProjectHandler final : CommandHandler<OpenProjectCommand> {
            OpenProjectHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(OpenProjectCommand&) override { _app.openProject(); }

        private:
            shell::ShellApp& _app;
        };

        struct CloseProjectHandler final : CommandHandler<CloseProjectCommand> {
            CloseProjectHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(CloseProjectCommand&) override { _app.closeProject(); }

        private:
            shell::ShellApp& _app;
        };

        struct ShowLogsHandler final : CommandHandler<ShowLogsCommand> {
            ShowLogsHandler(Workspace& workspace) : _workspace(workspace) { }

            void invoke(ShowLogsCommand&) override { _workspace.openEditor(shell::LogEditor::editorTypeId); }

        private:
            Workspace& _workspace;
        };

        struct ImportResourcesHandler final : CommandHandler<ImportResourcesCommand> {
            ImportResourcesHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(ImportResourcesCommand&) override { _app.importResources(); }

        private:
            shell::ShellApp& _app;
        };

        struct ShowAboutHandler final : CommandHandler<ShowAboutCommand> {
            ShowAboutHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(ShowAboutCommand&) override { _app.showAboutDialog(); }

        private:
            shell::ShellApp& _app;
        };

        struct ShowImguiDemoHandler final : CommandHandler<ShowImguiDemoCommand> {
            ShowImguiDemoHandler(shell::ShellApp& app) : _app(app) { }

            void invoke(ShowImguiDemoCommand&) override { _app.showImguiDemo(); }

        private:
            shell::ShellApp& _app;
        };

        struct CommandPaletteHandler final : CommandHandler<CommandPaletteCommand> {
            CommandPaletteHandler(Workspace& workspace) : _workspace(workspace) { }

            void invoke(CommandPaletteCommand&) override { _workspace.showPalette(); }

        private:
            Workspace& _workspace;
        };

        struct PlaySceneHandler final : CommandHandler<shell::PlaySceneCommand> {
            PlaySceneHandler(AudioEngine& audio, Workspace& workspace) : _audio(audio), _workspace(workspace) { }

            void invoke(shell::PlaySceneCommand& cmd) override {
                _workspace.createEditor<shell::GameEditor>(_audio, std::move(cmd.space));
            }

        private:
            AudioEngine& _audio;
            Workspace& _workspace;
        };
    } // namespace
} // namespace up

up::shell::ShellApp::ShellApp() : _logger("shell"), _propertyGrid(_assetLoader) { }

up::shell::ShellApp::~ShellApp() {
    _renderer.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();

    _reconClient.stop();
    _ioLoop.reset();
}

int up::shell::ShellApp::initialize() {
    TracyAppInfo("PotatoShell", stringLength("PotatoShell"));
    ZoneScopedN("Initialize Shell");

    zstring_view configPath = "shell.config.json";
    if (fs::fileExists(configPath)) {
        _loadConfig(configPath);
    }

    {
        char* prefPathSdl = SDL_GetPrefPath("potato", "shell");
        if (auto const rs = fs::createDirectories(prefPathSdl); rs != IOResult::Success) {
            _logger.error("Failed to create preferences folder `{}`", prefPathSdl);
        }
        _shellSettingsPath = path::join(prefPathSdl, "settings.json");
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        SDL_free(prefPathSdl);
    }

    schema::EditorSettings settings;
    if (loadShellSettings(_shellSettingsPath, settings)) {
        _logger.info("Loaded user settings: ", _shellSettingsPath);
    }

    {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags =
            ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_ViewportsEnable;
        io.ConfigInputTextCursorBlink = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigViewportsNoAutoMerge = true;
        io.ConfigDockingTransparentPayload = true;

        auto& style = ImGui::GetStyle();
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }

    _window = SDL_CreateWindow(
        "loading",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024,
        768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (_window == nullptr) {
        _errorDialog("Could not create window");
    }
    _updateTitle();

    SDL_DisplayMode desktopMode;
    int const displayIndex = SDL_GetWindowDisplayIndex(_window.get());
    int const displayResult = SDL_GetCurrentDisplayMode(displayIndex, &desktopMode);
    if (displayResult == 0) {
        int const minWidth = 640;
        int const minHeight = 480;
        auto const maxWidth = static_cast<int>(desktopMode.w * 0.8);
        auto const maxHeight = static_cast<int>(desktopMode.h * 0.8);
        int const ratioWidth = 3;
        int const ratioHeight = 2;

        int defaultWidth = maxWidth;
        int defaultHeight = maxHeight;

        if (defaultHeight < defaultWidth) {
            defaultWidth = clamp(defaultHeight / ratioHeight * ratioWidth, minWidth, maxWidth);
        }
        else {
            defaultHeight = clamp(defaultWidth / ratioWidth * ratioHeight, minHeight, maxHeight);
        }

        int const posX = static_cast<int>((desktopMode.w - defaultWidth) * 0.5);
        int const posY = static_cast<int>((desktopMode.h - defaultHeight) * 0.5);

        SDL_SetWindowSize(_window.get(), defaultWidth, defaultHeight);
        SDL_SetWindowPosition(_window.get(), posX, posY);
    }

    SDL_ShowWindow(_window.get());

    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);

    if (SDL_GetWindowWMInfo(_window.get(), &wmInfo) != SDL_TRUE) {
        _errorDialog("Could not get window info");
        return 1;
    }

    _audio = AudioEngine::create();
    _audio->registerAssetBackends(_assetLoader);

#if defined(UP_GPU_ENABLE_D3D11)
    if (_device == nullptr) {
        auto factory = CreateFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = CreateFactoryNull();
        _device = factory->createDevice(0);
    }

    if (_device == nullptr) {
        _errorDialog("Could not find device");
        return 1;
    }

    _renderer = new_box<Renderer>(_device);

    ImGui::Potato::LoadFonts();
    _device->initImgui(*ImGui::GetCurrentContext(), _window.get());
    ImGui::Potato::ApplyStyle();

#if UP_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        _errorDialog("Failed to create swap chain");
        return 1;
    }

    Material::registerLoader(_assetLoader, *_device);
    Mesh::registerLoader(_assetLoader, *_device);
    Shader::registerLoader(_assetLoader, *_device);
    Texture::registerLoader(_assetLoader, *_device);

    _sceneDatabase.registerComponent<TransformEditComponent>();
    _sceneDatabase.registerComponent<MeshEditComponent>();
    _sceneDatabase.registerComponent<WaveEditComponent>();
    _sceneDatabase.registerComponent<SpinEditComponent>();
    _sceneDatabase.registerComponent<DingEditComponent>();
    _sceneDatabase.registerComponent<BodyEditComponent>();
    _sceneDatabase.registerComponent<TestEditComponent>();

    AssetEditor::addFactory(_workspace, _assetLoader, _reconClient, _assetEditService, [this](UUID const& uuid) {
        _openAssetEditor(uuid);
    });
    SceneEditor::addFactory(_workspace, _sceneDatabase, _propertyGrid, _assetLoader);
    MaterialEditor::addFactory(_workspace, _propertyGrid);
    LogEditor::addFactory(_workspace, _logHistory);
    GameEditor::addFactory(_workspace, *_audio);

    _commands.addCommand<QuitCommand>();
    _commands.addCommand<OpenProjectCommand>();
    _commands.addCommand<CloseProjectCommand>();
    _commands.addCommand<ShowLogsCommand>();
    _commands.addCommand<ImportResourcesCommand>();
    _commands.addCommand<ShowAboutCommand>();
    _commands.addCommand<ShowImguiDemoCommand>();
    _commands.addCommand<CommandPaletteCommand>();

    _commandScope.addHandler<QuitHandler>(*this);
    _commandScope.addHandler<OpenProjectHandler>(*this);
    _commandScope.addHandler<CloseProjectHandler>(*this);
    _commandScope.addHandler<ShowLogsHandler>(_workspace);
    _commandScope.addHandler<ImportResourcesHandler>(*this);
    _commandScope.addHandler<ShowAboutHandler>(*this);
    _commandScope.addHandler<ShowImguiDemoHandler>(*this);
    _commandScope.addHandler<CommandPaletteHandler>(_workspace);
    _commandScope.addHandler<PlaySceneHandler>(*_audio, _workspace);

    Workspace::addCommands(_commands);
    GameEditor::addCommands(_commands);
    SceneEditor::addCommands(_commands);

    if (!settings.project.empty()) {
        _loadProject(settings.project);
    }

    return 0;
}

bool up::shell::ShellApp::_selectAndLoadProject(zstring_view folder) {
    nfdchar_t* selectedPath = nullptr;
    auto result = NFD_OpenDialog("popr", path::normalize(folder).c_str(), &selectedPath);
    if (result == NFD_ERROR) {
        result = NFD_OpenDialog("popr", nullptr, &selectedPath);
    }
    if (result == NFD_ERROR) {
        _logger.error("NDF_OpenDialog error: {}", NFD_GetError());
        return false;
    }
    if (result == NFD_CANCEL || selectedPath == nullptr) {
        return false;
    }
    bool success = _loadProject(selectedPath);
    free(selectedPath); // NOLINT(cppcoreguidelines-no-malloc)

    if (success) {
        schema::EditorSettings settings;
        settings.project = string{_project->projectFilePath()};
        if (!saveShellSettings(_shellSettingsPath, settings)) {
            _logger.error("Failed to save shell settings to `{}`", _shellSettingsPath);
        }
    }
    return success;
}

bool up::shell::ShellApp::_loadProject(zstring_view path) {
    _logger.info("Loading project: {}", path);

    _project = Project::loadFromFile(path);
    if (_project == nullptr) {
        _errorDialog("Could not load project file");
        return false;
    }

    _projectName = string{path::filebasename(path)};
    _assetEditService.setAssetRoot(string{_project->resourceRootPath()});

    _loadManifest();

    _workspace.closeAll();
    _workspace.openEditor(AssetEditor::editorTypeId);
    _updateTitle();

    if (!_reconClient.start(_ioLoop, _project->resourceRootPath())) {
        _logger.error("Failed to start recon");
    }
    _reconClient.on<ReconManifestMessage>([this](auto const&) { _loadManifest(); });

    return true;
}

void up::shell::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    auto now = std::chrono::high_resolution_clock::now();
    _lastFrameDuration = now - now;

    int width = 0;
    int height = 0;

    constexpr double nano_to_seconds = 1.0 / 1000000000.0;

    while (isRunning()) {
        ZoneScopedN("Main Loop");

        imguiIO.DeltaTime = _lastFrameTime;

        {
            ZoneScopedN("I/O");
            _ioLoop.run(IORun::Poll);
        }

        _processEvents();

        if (_openProject && !_closeProject) {
            ZoneScopedN("Load Project");
            _openProject = false;
            if (!_selectAndLoadProject(
                    path::join(fs::currentWorkingDirectory(), "..", "..", "..", "..", "resources"))) {
                continue;
            }
        }

        SDL_GetWindowSize(_window.get(), &width, &height);
        imguiIO.DisplaySize.x = static_cast<float>(width);
        imguiIO.DisplaySize.y = static_cast<float>(height);

        _device->beginImguiFrame();

        _displayUI();

        ImGui::EndFrame();

        _render();

        _assetLoader.collectDoomedAssets();

        if (_closeProject) {
            _reconClient.stop();
            _closeProject = false;
            _workspace.closeAll();
            _project = nullptr;
            _updateTitle();
        }

        auto endFrame = std::chrono::high_resolution_clock::now();
        _lastFrameDuration = endFrame - now;
        _lastFrameTime = static_cast<float>(static_cast<double>(_lastFrameDuration.count()) * nano_to_seconds);
        now = endFrame;
    }
}

void up::shell::ShellApp::quit() {
    _running = false;
}

void up::shell::ShellApp::openProject() {
    _openProject = true;
    _closeProject = true;
}

void up::shell::ShellApp::closeProject() {
    _closeProject = true;
}

void up::shell::ShellApp::importResources() {
    _executeRecon();
}

void up::shell::ShellApp::_onWindowClosed() {
    quit();
}

void up::shell::ShellApp::_onWindowSizeChanged() {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _swapChain->resizeBuffers(width, height);

    _logger.info("Window resized: {}x{}", width, height);
}

void up::shell::ShellApp::_updateTitle() {
    static constexpr char appName[] = "Potato Shell";

    if (_project == nullptr) {
        SDL_SetWindowTitle(_window.get(), appName);
        return;
    }

    char title[128] = {};
    nanofmt::format_to(title, "{} [{}]", appName, _projectName);
    SDL_SetWindowTitle(_window.get(), title);
}

void up::shell::ShellApp::_processEvents() {
    ZoneScopedN("Shell Events");

    // TODO: https://github.com/potatoengine/potato/issues/305
    // SDL_SetRelativeMouseMode(ImGui::IsCaptureRelativeMouseMode() ? SDL_TRUE : SDL_FALSE);

    SDL_Event ev;
    while (_running && SDL_PollEvent(&ev) > 0) {
        switch (ev.type) {
            case SDL_QUIT:
                quit();
                break;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        _onWindowClosed();
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        _onWindowSizeChanged();
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                    case SDL_WINDOWEVENT_EXPOSED:
                        break;
                }
                _device->handleImguiEvent(ev);
                break;
            case SDL_KEYDOWN:
                _commands.pushScope(_commandScope);
                if (!_workspace.evaluateHotkey(_commands, ev.key.keysym.sym, ev.key.keysym.mod)) {
                    _device->handleImguiEvent(ev);
                }
                _commands.popScope(_commandScope);
                break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEWHEEL:
            default:
                _device->handleImguiEvent(ev);
                break;
        }
    }
}

void up::shell::ShellApp::_render() {
    ZoneScopedN("Shell Render");

    _renderer->beginFrame();
    _device->renderImgui(*_swapChain);
    _swapChain->present();
    FrameMark;
}

void up::shell::ShellApp::_displayUI() {
    ZoneScopedN("Shell UI");

    _commands.pushScope(_commandScope);

    _workspace.update(*_renderer, _commands, _lastFrameTime);

    if (_aboutDialog) {
        ImGui::SetNextWindowSizeConstraints({400, 300}, {});
        ImGui::Begin("About", &_aboutDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Potato editor");
        ImGui::End();
    }

    _commands.popScope(_commandScope);

    if (_imguiDemo) {
        ImGui::ShowDemoWindow(&_imguiDemo);
    }
}

void up::shell::ShellApp::_errorDialog(zstring_view message) {
    _logger.error("Fatal error: {}", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), _window.get());
}

bool up::shell::ShellApp::_loadConfig(zstring_view path) {
    auto [rs, jsonRoot] = readJson(path);
    if (rs != IOResult{} || !jsonRoot.is_object()) {
        _logger.error("Failed to parse file `{}': {}", path, rs);
        return false;
    }

    return true;
}

void up::shell::ShellApp::_openAssetEditor(UUID const& uuid) {
    string assetPath;
    uint64 assetTypeHash = 0;
    for (auto const& record : _assetLoader.manifest()->records()) {
        if (record.uuid == uuid) {
            assetPath = path::join(_project->resourceRootPath(), record.filename);
            assetTypeHash = hash_value(record.type);
            break;
        }
    }
    if (assetPath.empty()) {
        return;
    }

    EditorTypeId const editor = _assetEditService.findInfoForAssetTypeHash(assetTypeHash).editor;
    if (editor.valid()) {
        _workspace.openEditorForDocument(editor, assetPath);
    }
    else {
        if (!desktop::openInExternalEditor(assetPath)) {
            _logger.error("Failed to open application for asset: {}", assetPath);
        }
    }
}

void up::shell::ShellApp::_executeRecon() {
    _reconClient.send<ReconImportAllMessage>({.force = true});

    _loadManifest();
}

void up::shell::ShellApp::_loadManifest() {
    ZoneScopedN("Load Manifest");
    string manifestPath = path::join(_project->libraryPath(), "manifest.txt");
    _logger.info("Loading manifest {}", manifestPath);
    if (auto [rs, manifestText] = fs::readText(manifestPath); rs == IOResult{}) {
        auto manifest = new_box<ResourceManifest>();
        if (!ResourceManifest::parseManifest(manifestText, *manifest)) {
            _logger.error("Failed to parse resource manifest");
        }
        string casPath = path::join(_project->libraryPath(), "cache");
        _assetLoader.bindManifest(std::move(manifest), std::move(casPath));
    }
    else {
        _logger.error("Failed to load resource manifest");
    }
}
