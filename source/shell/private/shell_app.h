// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/pipeline_state.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/resource_view.h"
#include "grimm/imgrui/imgrui.h"
#include "grimm/render/renderer.h"
#include "grimm/render/node.h"

#include <SDL.h>

namespace gm {
    class ShellApp;
}

class gm::ShellApp {
public:
    ShellApp();
    ~ShellApp();

    ShellApp(ShellApp const&) = delete;
    ShellApp& operator=(ShellApp const&) = delete;

    int initialize();
    void run();
    void quit();

    bool isRunning() const { return _running; }

private:
    void onWindowSizeChanged();
    void onWindowClosed();

private:
    bool _running = true;
    fs::FileSystem _fileSystem;
    rc<gpu::Device> _device;
    rc<gpu::SwapChain> _swapChain;
    box<gpu::CommandList> _commandList;
    box<Renderer> _renderer;
    box<Camera> _camera;
    box<Node> _root;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    imgrui::DrawImgui _drawImgui;
};
