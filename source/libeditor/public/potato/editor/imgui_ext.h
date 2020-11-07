// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "icons.h"

#include <glm/fwd.hpp>
#include <imgui.h>

namespace ImGui::inline Potato {
    UP_EDITOR_API bool MenuItemEx(
        const char* label,
        const char8_t* icon = nullptr,
        const char* shortcut = nullptr,
        bool selected = false,
        bool enabled = true);

    UP_EDITOR_API bool IconButton(
        char const* label,
        char8_t const* icon,
        ImVec2 size = {},
        ImGuiButtonFlags flags = ImGuiButtonFlags_None);

    UP_EDITOR_API void SetCaptureRelativeMouseMode(bool captured);
    UP_EDITOR_API auto IsCaptureRelativeMouseMode() -> bool;

    UP_EDITOR_API bool InputVec3(
        char const* label,
        glm::vec3& value,
        char const* format = "%.3f",
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    UP_EDITOR_API bool InputQuat(
        char const* label,
        glm::quat& value,
        char const* format = "%.3f",
        ImGuiSliderFlags flags = ImGuiSliderFlags_None);
} // namespace ImGui::inline Potato
