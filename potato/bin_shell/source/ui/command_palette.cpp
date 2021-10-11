// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/command_palette.h"
#include "ui/action.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/sequence.h"
#include "potato/spud/sort.h"

#include <SDL_keycode.h>
#include <imgui.h>
#include <imgui_internal.h>

void up::shell::CommandPalette::bindActions(Actions& actions) {
    _commandActions.addAction(
        {.name = "potato.editor.command-palette.open"_s,
         .menu = "Edit\\Commands",
         .hotKey = "Ctrl+Shift+P"_s,
         .action = delegate(this, &CommandPalette::show)});
    actions.addGroup(&_commandActions);

    _actions = &actions;
}

void up::shell::CommandPalette::show() {
    _open = true;
}

void up::shell::CommandPalette::close() {
    _input[0] = '\0';
    _activeIndex = 0;
    _matches.clear();
    _open = false;
}

void up::shell::CommandPalette::drawPalette() {
    auto& imguiIO = ImGui::GetIO();

    auto const popupName = "##CommandInput";
    auto const popupFlags = ImGuiWindowFlags_NoMove;

    // Handle opening of dialog
    //
    if (_open) {
        ImGui::OpenPopup(popupName);
    }

    // Measure width of the menu bar, used to offset the paletter
    //
    ImGui::BeginMainMenuBar();
    auto const menuSize = ImGui::GetWindowSize();
    ImGui::EndMainMenuBar();

    // Position and display the popup
    //
    auto const halfScreenWidth = imguiIO.DisplaySize.x * 0.5f;
    auto const dialogWidth = max(halfScreenWidth, 220.f);

    ImGui::SetNextWindowPos({halfScreenWidth - dialogWidth * 0.5f, menuSize.y});
    ImGui::SetNextWindowSize({dialogWidth, 0});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const isOpen = ImGui::BeginPopup(popupName, popupFlags);
    ImGui::PopStyleVar(1);

    // Render popup contents only if open
    //
    if (isOpen) {
        _rebuild();

        auto const inputName = "##command";
        auto const inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
            ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

        // Display input
        //
        ImGui::SetNextItemWidth(dialogWidth);
        if (ImGui::IsWindowAppearing()) { // I cannot get SetItemDefaultFocus to work
            ImGui::SetKeyboardFocusHere();
        }

        auto const activated = ImGui::InputText(inputName, _input, sizeof(_input), inputFlags, &_callbackWrapper, this);

        // Update active index based on current input _before_ executing input
        //
        _updateMatches();

        // Execute input if the input was activated (Enter pressed)
        //
        if (activated) {
            if (_input[0] == '\0' || _execute()) {
                close();
            }

            // Ensure input remains active if Enter was pressed but no command was executed
            //
            if (_open) {
                ImGui::SetActiveID(ImGui::GetID(inputName), ImGui::GetCurrentContext()->CurrentWindow);
            }
        }

        // Unset our open state if the popup is closed
        //
        if (_open && !activated && ImGui::IsItemDeactivated()) {
            close();
        }

        // Draw matching commands
        //
        if (!_matches.empty()) {
            ImGui::BeginColumns("##commands", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
            auto const hotKeyColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
            for (auto const& [index, match] : enumerate(_matches)) {
                bool const highlight = index == _activeIndex;
                auto const& action = _actions->actionAt(_commands[match].id);
                if (index != 0) {
                    ImGui::NextColumn();
                }
                ImGui::Selectable(
                    action.command.c_str(),
                    highlight,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups);
                ImGui::NextColumn();
                ImGui::TextColored(hotKeyColor, "%s", action.hotKey.c_str());
            }
            ImGui::EndColumns();
            ImGui::Spacing();
        }

        // Close popup if close is requested
        //
        if (!_open) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

auto up::shell::CommandPalette::_callbackWrapper(ImGuiInputTextCallbackData* data) -> int {
    return static_cast<CommandPalette*>(data->UserData)->_callback(data);
}

void up::shell::CommandPalette::_rebuild() {
    if (_actions == nullptr || !_actions->refresh(_actionsVersion)) {
        return;
    }

    _commands.clear();

    _actions->build([this](auto const id, auto const& action) {
        if (action.command.empty()) {
            return;
        }

        _commands.push_back({.id = id});
    });

    sort(_commands, {}, [this](Command const& cmd) noexcept { return _actions->actionAt(cmd.id).command; });
}

auto up::shell::CommandPalette::_callback(ImGuiInputTextCallbackData* data) -> int {
    switch (data->EventKey) {
        case ImGuiKey_Tab:
            return 0;
        case ImGuiKey_UpArrow:
            --_activeIndex;
            return 0;
        case ImGuiKey_DownArrow:
            ++_activeIndex;
            return 0;
        default:
            return 0;
    }
}

auto up::shell::CommandPalette::_execute() const -> bool {
    if (_activeIndex >= _matches.size()) {
        return false;
    }

    auto const commandIndex = _matches[_activeIndex];
    auto const& command = _commands[commandIndex];

    _actions->invoke(command.id);
    return true;
}

void up::shell::CommandPalette::_updateMatches() {
    _matches.clear();

    if (_input[0] == '\0') {
        _activeIndex = 0;
        return;
    }
    auto const input = string_view{_input};

    for (auto const& [index, command] : enumerate(_commands)) {
        zstring_view cmd = _actions->actionAt(command.id).command;

        if (stringIndexOfNoCase(cmd.data(), cmd.size(), input.data(), input.size()) < 0) {
            continue;
        }

        if (!_actions->isEnabled(command.id)) {
            continue;
        }

        _matches.push_back(index);
    }

    if (_matches.empty()) {
        _activeIndex = 0;
    }

    if (_activeIndex >= _matches.size()) {
        _activeIndex = _matches.size() - 1;
    }
}
