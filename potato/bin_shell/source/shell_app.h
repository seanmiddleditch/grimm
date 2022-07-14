// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "settings.h"

#include "potato/audio/audio_engine.h"
#include "potato/editor/asset_edit_service.h"
#include "potato/editor/command.h"
#include "potato/editor/workspace.h"
#include "potato/editor/property_grid.h"
#include "potato/recon/recon_client.h"
#include "potato/shell/log_history.h"
#include "potato/shell/scene_doc.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/io_loop.h"
#include "potato/runtime/logger.h"
#include "potato/spud/box.h"
#include "potato/spud/unique_resource.h"

#include <SDL.h>
#include <chrono>
#include <imgui.h>

namespace up {
    class Renderer;
    class Node;
    class Model;
    class GpuDevice;
    class GpuSwapChain;
    class GpuTexture;
    class Camera;
    class CameraController;
    class Project;
    class UUID;
} // namespace up

namespace up::shell {
    class ShellApp {
    public:
        ShellApp();
        ~ShellApp();

        ShellApp(ShellApp const&) = delete;
        ShellApp& operator=(ShellApp const&) = delete;

        int initialize();
        void run();
        void quit();
        void openProject();
        void closeProject();
        void importResources();

        void showAboutDialog() { _aboutDialog = true; }
        void showImguiDemo() { _imguiDemo = true; }

        bool isRunning() const { return _running; }

    private:
        void _onWindowSizeChanged();
        void _onWindowClosed();

        void _updateTitle();
        void _processEvents();
        void _render();

        void _displayUI();

        void _errorDialog(zstring_view message);

        bool _loadConfig(zstring_view path);

        void _openAssetEditor(UUID const& uuid);

        void _executeRecon();
        void _loadManifest();

        bool _selectAndLoadProject(zstring_view folder);
        bool _loadProject(zstring_view path);

        bool _running = true;
        bool _openProject = false;
        bool _closeProject = false;
        bool _aboutDialog = false;
        bool _imguiDemo = false;
        rc<GpuDevice> _device;
        rc<GpuSwapChain> _swapChain;
        box<Renderer> _renderer;
        box<AudioEngine> _audio;
        box<Project> _project;
        SceneDatabase _sceneDatabase;
        string _shellSettingsPath;
        unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
        unique_resource<SDL_Cursor*, SDL_FreeCursor> _cursor;
        Workspace _workspace;
        CommandManager _commands;
        CommandScope _commandScope;
        Logger _logger;
        float _lastFrameTime = 0.f;
        std::chrono::nanoseconds _lastFrameDuration = {};
        string _projectName;
        AssetLoader _assetLoader;
        PropertyGrid _propertyGrid;
        LogHistory _logHistory;
        ReconClient _reconClient;
        AssetEditService _assetEditService;
        IOLoop _ioLoop;
    }; // namespace up::shell
} // namespace up::shell
