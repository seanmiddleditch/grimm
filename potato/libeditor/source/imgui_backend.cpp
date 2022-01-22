// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/imgui_backend.h"

#include "potato/data/fontawesome_font.h"
#include "potato/data/roboto_font.h"
#include "potato/render/context.h"
#include "potato/render/gpu_device.h"
#include "potato/runtime/assertion.h"

#include <SDL_clipboard.h>
#include <SDL_events.h>
#include <imgui.h>

up::ImguiBackend::ImguiBackend() : _context(ImGui::CreateContext()) {
    ImGui::SetCurrentContext(_context.get());
    auto& io = ImGui::GetIO();

    _loadFonts();
    _applyStyle();

    io.BackendPlatformName = "potato";
    io.UserData = this;
    io.IniFilename = nullptr;

    io.ConfigFlags = ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigInputTextCursorBlink = true;

    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

    io.SetClipboardTextFn = _setClipboardTextContents;
    io.GetClipboardTextFn = _getClipboardTextContents;
    io.ClipboardUserData = this;
}

up::ImguiBackend::~ImguiBackend() = default;

void up::ImguiBackend::beginFrame(GpuDevice& device) {
    ImGui::SetCurrentContext(_context.get());
    device.beginImguiFrame(*_context.get());
    ImGui::NewFrame();
    _captureRelativeMouseMode = false;
}

void up::ImguiBackend::endFrame() {
    ImGui::SetCurrentContext(_context.get());
    ImGui::EndFrame();
}

bool up::ImguiBackend::handleEvent(SDL_Event const& ev) {
    UP_GUARD(!_context.empty(), false);

    ImGui::SetCurrentContext(_context.get());
    ImGuiIO& io = ImGui::GetIO();

    auto const toImguiButton = [](int sdlButton) noexcept {
        switch (sdlButton) {
            case 1:
                return 0;
            case 2:
                return 2;
            case 3:
                return 1;
            default:
                return 0;
        }
    };

    switch (ev.type) {
        case SDL_MOUSEMOTION:
            io.MousePos = {(float)ev.motion.x, (float)ev.motion.y};
            return io.WantCaptureMouse;
        case SDL_MOUSEBUTTONDOWN:
            io.MouseDown[toImguiButton(ev.button.button)] = true;
            return io.WantCaptureMouse;
        case SDL_MOUSEBUTTONUP:
            io.MouseDown[toImguiButton(ev.button.button)] = false;
            return io.WantCaptureMouse;
        case SDL_MOUSEWHEEL:
            if (ev.wheel.y > 0) {
                io.MouseWheel += 1;
            }
            else if (ev.wheel.y < 0) {
                io.MouseWheel -= 1;
            }

            if (ev.wheel.x > 0) {
                io.MouseWheelH += 1;
            }
            else if (ev.wheel.x < 0) {
                io.MouseWheelH -= 1;
            }
            return io.WantCaptureMouse;
        case SDL_TEXTINPUT:
            io.AddInputCharactersUTF8(ev.text.text);
            return io.WantTextInput;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            {
                int key = ev.key.keysym.scancode;
                IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
                io.KeysDown[key] = (ev.type == SDL_KEYDOWN);
                io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
                io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
                io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
                io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
                return io.WantCaptureKeyboard;
            }
    }
    return false;
}

void up::ImguiBackend::render(GpuDevice& device, GpuSwapChain& swapChain) {
    ImGui::Render();
    device.renderImgui(*_context.get(), swapChain);

    ImGui::SetCurrentContext(_context.get());
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

char const* up::ImguiBackend::_getClipboardTextContents(void* self) {
    auto imgui = static_cast<ImguiBackend*>(self);
    imgui->_clipboardTextData = up::string(SDL_GetClipboardText());
    return imgui->_clipboardTextData.c_str();
}

void up::ImguiBackend::_setClipboardTextContents(void* self, char const* zstr) {
    auto imgui = static_cast<ImguiBackend*>(self);
    imgui->_clipboardTextData.reset();
    SDL_SetClipboardText(zstr);
}

ImFont* up::ImguiBackend::getFont(int index) const noexcept {
    UP_GUARD(index >= 0 && index < static_cast<int>(ImGui::UpFont::Count_), _fonts[0]);
    return _fonts[index];
}

void up::ImguiBackend::_loadFonts() {
    auto& io = ImGui::GetIO();

    ImFontConfig config;
    config.MergeMode = false;
    config.PixelSnapH = false;
    config.FontDataOwnedByAtlas = false;

    _fonts[(int)ImGui::UpFont::Roboto_16] =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(roboto_font_data), roboto_font_size, 16.f, &config);

    config.MergeMode = true;
    config.GlyphMinAdvanceX = 14.f;
    config.PixelSnapH = true;
    config.FontDataOwnedByAtlas = false;

    static constexpr auto fontawesomeMinGlyph = 0xf000;
    static constexpr auto fontawesomeMaxGlyph = 0xf897;
    static constexpr ImWchar s_ranges[] = {fontawesomeMinGlyph, fontawesomeMaxGlyph, 0};

    io.Fonts->AddFontFromMemoryTTF(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<unsigned char*>(fontawesome_font_data),
        fontawesome_font_size,
        12.f,
        &config,
        s_ranges);

    config.MergeMode = false;
    config.GlyphMinAdvanceX = 72.f;

    _fonts[(int)ImGui::UpFont::FontAwesome_72] = io.Fonts->AddFontFromMemoryTTF(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<unsigned char*>(fontawesome_font_data),
        fontawesome_font_size,
        72.f,
        &config,
        s_ranges);
}

void up::ImguiBackend::_applyStyle() {
    // From: https://github.com/ocornut/imgui/issues/707#issuecomment-512669512

    auto& style = ImGui::GetStyle();

    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.PopupRounding = 2.0f;
    style.ChildRounding = 2.0f;

    style.WindowPadding = ImVec2(4.0f, 4.0f);
    style.FramePadding = ImVec2(4.0f, 4.0f);

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.GrabMinSize = 18.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f); // ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void up::ImguiBackend::_freeContext(ImGuiContext* ctx) {
    if (ctx != nullptr) {
        ImGui::DestroyContext(ctx);
    }
}
