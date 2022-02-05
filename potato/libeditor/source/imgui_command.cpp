// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/imgui_command.h"

#include "potato/editor/command.h"
#include "potato/editor/imgui_ext.h"

#include <imgui_internal.h>

namespace ImGui::inline Potato {
    bool CommandButton(
        up::CommandManager& commands,
        up::CommandId command,
        char const* label,
        char const* icon,
        ImVec2 size,
        ImGuiButtonFlags flags) {
        if (label == nullptr || icon == nullptr) {
            up::CommandMeta const* const meta = commands.fetchMetadata(command);
            if (label == nullptr) {
                label = meta != nullptr ? meta->displayName.c_str() : "<Command>";
            }
            if (icon == nullptr && meta != nullptr) {
                icon = meta->icon.c_str();
            }
        }

        bool disabled = !commands.condition(command);

        auto const status = commands.status(command);
        if ((status & up::CommandStatus::Disabled) == up::CommandStatus::Disabled) {
            disabled = true;
        }

        ImGui::BeginDisabled(disabled);

        bool clicked = false;
        if (icon != nullptr) {
            clicked = ImGui::Potato::IconButton(label, icon, size, flags);
        }
        else {
            clicked = ImGui::ButtonEx(label, size, flags);
        }

        ImGui::EndDisabled();

        return clicked && !disabled;
    }

    bool CommandMenuItem(up::CommandManager& commands, up::CommandId command, const char* label, const char* icon) {
        up::CommandMeta const* const meta = commands.fetchMetadata(command);
        char const* hotkey = meta != nullptr && !meta->hotkey.empty() ? meta->hotkey.c_str() : nullptr;
        if (label == nullptr) {
            label = meta != nullptr ? meta->displayName.c_str() : "<Command>";
        }
        if (icon == nullptr && meta != nullptr) {
            icon = meta->icon.c_str();
        }

        bool disabled = !commands.condition(command);

        auto const status = commands.status(command);
        if ((status & up::CommandStatus::Disabled) == up::CommandStatus::Disabled) {
            disabled = true;
        }

        bool checked = (status & up::CommandStatus::Checked) == up::CommandStatus::Checked;

        if (icon != nullptr) {
            return ImGui::MenuItemEx(label, icon, hotkey, checked, !disabled);
        }
        return ImGui::MenuItem(label, hotkey, &checked, !disabled);
    }
} // namespace ImGui::inline Potato
