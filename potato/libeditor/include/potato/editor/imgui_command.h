// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/editor/editor_common.h"

#include <glm/fwd.hpp>
#include <imgui.h>

namespace up {
    class CommandManager;
}

namespace ImGui::inline Potato {
    UP_EDITOR_API bool CommandButton(
        up::CommandManager& commands,
        up::CommandId command,
        char const* label = nullptr,
        char const* icon = nullptr,
        ImVec2 size = {},
        ImGuiButtonFlags flags = ImGuiButtonFlags_None);

    UP_EDITOR_API bool CommandMenuItem(
        up::CommandManager& commands,
        up::CommandId command,
        const char* label = nullptr,
        const char* icon = nullptr);
} // namespace ImGui::inline Potato
