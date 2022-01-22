// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "imgui_fonts.h"

#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/unique_resource.h"

namespace up {
    class GpuDevice;
    class GpuSwapChain;
} // namespace up

struct ImDrawData;
struct ImDrawList;
struct ImData;
struct ImGuiContext;
struct ImGuiIO;
struct ImFont;
using SDL_Event = union SDL_Event;

namespace up {
    class Shader;

    class ImguiBackend {
    public:
        UP_EDITOR_API ImguiBackend();
        UP_EDITOR_API ~ImguiBackend();

        UP_EDITOR_API bool handleEvent(SDL_Event const& ev);

        UP_EDITOR_API void beginFrame(GpuDevice& device);
        UP_EDITOR_API void endFrame();

        UP_EDITOR_API void render(GpuDevice& device, GpuSwapChain& swapChain);

        void setCaptureRelativeMouseMode(bool captured) noexcept { _captureRelativeMouseMode = captured; }
        auto isCaptureRelativeMouseMode() const noexcept -> bool { return _captureRelativeMouseMode; }

        ImFont* getFont(int index) const noexcept;

    private:
        void _loadFonts();
        void _applyStyle();

        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext> _context;
        string _clipboardTextData;
        bool _captureRelativeMouseMode = false;
        ImFont* _fonts[static_cast<int>(ImGui::Potato::UpFont::Count_)] = {};
    };
} // namespace up
