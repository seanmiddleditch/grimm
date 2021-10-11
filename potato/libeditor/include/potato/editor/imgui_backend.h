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
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class RenderContext;
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

        UP_EDITOR_API bool createResources(GpuDevice& device);
        UP_EDITOR_API void releaseResources();

        UP_EDITOR_API bool handleEvent(SDL_Event const& ev);

        UP_EDITOR_API void beginFrame();
        UP_EDITOR_API void endFrame();

        UP_EDITOR_API void render(RenderContext& ctx);

        void setCaptureRelativeMouseMode(bool captured) noexcept { _captureRelativeMouseMode = captured; }
        auto isCaptureRelativeMouseMode() const noexcept -> bool { return _captureRelativeMouseMode; }

        ImFont* getFont(int index) const noexcept;

    private:
        void _initialize();
        void _ensureContext();
        void _loadFonts();
        void _applyStyle();

        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext> _context;
        box<GpuBuffer> _indexBuffer;
        box<GpuBuffer> _vertexBuffer;
        box<GpuBuffer> _constantBuffer;
        box<GpuPipelineState> _pipelineState;
        box<GpuResourceView> _srv;
        box<GpuSampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
        string _clipboardTextData;
        bool _captureRelativeMouseMode = false;
        ImFont* _fonts[static_cast<int>(ImGui::Potato::UpFont::Count_)] = {};
    };
} // namespace up
