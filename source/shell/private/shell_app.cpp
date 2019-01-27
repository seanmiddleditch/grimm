// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/vector.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/stream_util.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/factory.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/texture.h"
#include "grimm/math/packed.h"
#include "grimm/render/camera.h"
#include "grimm/render/node.h"
#include "grimm/render/model.h"
#include "grimm/render/material.h"
#include "grimm/imgrui/imgrui.h"

#include <fmt/chrono.h>
#include <chrono>
#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <imgui.h>

gm::ShellApp::ShellApp() = default;

gm::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _commandList.reset();
    _root.reset();
    _camera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int gm::ShellApp::initialize() {
    using namespace gm;

    _window = SDL_CreateWindow("Grimm Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create window", nullptr);
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_window.get(), &wmInfo)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not get window info", _window.get());
    }

#if GM_GPU_ENABLE_D3D11
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryNull();
        _device = factory->createDevice(0);
    }

    if (_device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }

    _renderer = make_box<Renderer>(_device);

#if GM_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Failed to create swap chain", _window.get());
        return 1;
    }

    _camera = make_box<Camera>(_swapChain);

    _commandList = _device->createCommandList();
    if (_commandList == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create command list", _window.get());
        return 1;
    }

    blob basicVertShader, basicPixelShader;
    auto stream = _fileSystem.openRead("build/resources/shaders/basic.vs_5_0.cbo");
    if (fs::readBlob(stream, basicVertShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open vertex shader", _window.get());
        return 1;
    }
    stream = _fileSystem.openRead("build/resources/shaders/basic.ps_5_0.cbo");
    if (fs::readBlob(stream, basicPixelShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open pixel shader", _window.get());
        return 1;
    }

    auto material = make_shared<Material>(std::move(basicVertShader), std::move(basicPixelShader));
    auto model = make_box<Model>(std::move(material));
    _root = make_box<Node>(std::move(model));

    blob imguiVertShader, imguiPixelShader;
    stream = _fileSystem.openRead("build/resources/shaders/imgui.vs_5_0.cbo");
    if (fs::readBlob(stream, imguiVertShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open imgui vertex shader", _window.get());
        return 1;
    }
    stream = _fileSystem.openRead("build/resources/shaders/imgui.ps_5_0.cbo");
    if (fs::readBlob(stream, imguiPixelShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open imgui pixel shader", _window.get());
        return 1;
    }

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    _drawImgui.createResources(*_device);

    return 0;
}

void gm::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    std::chrono::high_resolution_clock clock;

    auto now = clock.now();
    auto duration = now - now;

    while (isRunning()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    onWindowClosed();
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    onWindowSizeChanged();
                    break;
                }
                break;
            }
            _drawImgui.handleEvent(ev);
        }

        gpu::Viewport viewport;
        int width, height;
        SDL_GetWindowSize(_window.get(), &width, &height);
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);

        imguiIO.DisplaySize.x = viewport.width;
        imguiIO.DisplaySize.y = viewport.height;
        _drawImgui.beginFrame();
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Grimm")) {
                if (ImGui::MenuItem("Quit")) {
                    return;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::Begin("Statistics")) {
            auto micro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            fixed_string_writer<128> buffer;
            format_into(buffer, "{}us", micro);
            ImGui::LabelText("Frametime", buffer.c_str());
            buffer.clear();
            format_into(buffer, "{}", 1000000.0 / micro);
            ImGui::LabelText("FPS", buffer.c_str());
        }
        ImGui::End();

        _camera->beginFrame(*_commandList, *_device);
        _root->render(*_commandList, *_device);

        _drawImgui.endFrame(*_device, *_commandList);

        _camera->endFrame(*_commandList, *_device);

        auto endFrame = clock.now();
        duration = endFrame - now;
        now = endFrame;
    }
}

void gm::ShellApp::quit() {
    _running = false;
}

void gm::ShellApp::onWindowClosed() {
    quit();
}

void gm::ShellApp::onWindowSizeChanged() {
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _camera->resetSwapChain(nullptr);
    _commandList->clear();
    _swapChain->resizeBuffers(width, height);
    _camera->resetSwapChain(_swapChain);
}
