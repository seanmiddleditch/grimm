// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/command_palette.h"

#include "potato/editor/command.h"
#include "potato/editor/icons.h"
#include "potato/editor/workspace.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/sequence.h"
#include "potato/spud/sort.h"

#include <SDL_keycode.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace up {
    struct PaletteInputState {
        CommandId prevId;
        CommandMeta const* prevMeta = nullptr;
        CommandManager* commands = nullptr;
        CommandPaletteState* paletteState = nullptr;
        bool wantPrevious = false;
        bool wantNext = false;
    };

    static bool showPaletteItem(CommandMeta const& meta, CommandId id, bool highlight) noexcept {
        auto const hotkeyColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(
                meta.displayName.c_str(),
                highlight,
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups)) {
            return true;
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(hotkeyColor, "%s", meta.hotkey.c_str());
        return false;
    };

    static int inputCallback(ImGuiInputTextCallbackData* data) noexcept {
        auto* const state = static_cast<PaletteInputState*>(data->UserData);
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
            if (data->EventKey == ImGuiKey_UpArrow) {
                state->wantPrevious = true;
            }
            else {
                state->wantNext = true;
            }
        }
        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion && state->commands != nullptr &&
            state->paletteState->activeId) {
            if (CommandMeta const* const meta = state->commands->fetchMetadata(state->paletteState->activeId);
                meta != nullptr) {
                auto const len = min(data->BufSize, static_cast<int>(meta->displayName.size()));
                std::memcpy(data->Buf, meta->displayName.data(), len);
                data->Buf[len] = '\0';
                data->BufTextLen = len;
                data->CursorPos = len;
                data->BufDirty = true;
            }
        }
        return 0;
    }

    static bool matchCommand(string_view input, CommandMeta const& meta) noexcept {
        return stringIndexOfNoCase(meta.displayName.data(), meta.displayName.size(), input.data(), input.size()) >= 0;
    }

    void showCommandPalette(CommandManager& commands, CommandPaletteState& state) {
        constexpr auto popupName = "##PotatoCommandPalettePopup";
        constexpr auto popupFlags = ImGuiWindowFlags_NoDecoration;

        // Handle opening of dialog
        //
        if (state.wantOpen) {
            state.wantOpen = false;
            if (!ImGui::IsPopupOpen(popupName)) {
                state.activeId = CommandId{};
                state.input[0] = '\0';
                ImGui::OpenPopup(popupName);
            }
        }

        // Position and display the popup
        //
        ImGuiWindow const* const window = ImGui::GetCurrentWindowRead();
        auto const dialogWidth = clamp(window->SizeFull.x * 0.5f, 220.f, 680.f);

        ImGui::SetNextWindowPos(
            window->Pos + ImVec2{window->Size.x * 0.5f - dialogWidth * 0.5f, ImGui::GetFrameHeight()});
        ImGui::SetNextWindowSize({dialogWidth, 0});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {4, 4});
        auto const isOpen = ImGui::BeginPopup(popupName, popupFlags);
        ImGui::PopStyleVar(1);

        // Render popup contents only if open
        //
        if (!isOpen) {
            state.input[0] = '\0';
            state.activeId = {};
            return;
        }

        auto const inputName = "##command";
        auto const inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
            ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

        // Display input
        //
        ImGui::SetNextItemWidth(-1.f);
        if (ImGui::IsWindowAppearing()) { // I cannot get SetItemDefaultFocus to work
            ImGui::SetKeyboardFocusHere();
        }

        PaletteInputState inputState{.commands = &commands, .paletteState = &state};
        auto const doExecute =
            ImGui::InputText(inputName, state.input, sizeof(state.input), inputFlags, &inputCallback, &inputState);
        bool const deactivated = ImGui::IsItemDeactivated();

        // Execute input if the input was activated (Enter pressed)
        //
        if (doExecute) {
            if (state.activeId && commands.invoke(state.activeId)) {
                ImGui::CloseCurrentPopup();
            }
            else {
                // Ensure input remains active if Enter was pressed but no command was executed
                //
                ImGui::SetActiveID(ImGui::GetID(inputName), ImGui::GetCurrentContext()->CurrentWindow);
            }
        }

        // Draw matching commands
        //
        ImGui::BeginTable("##commands", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX);
        ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_None);

        commands.each([&state, &commands, &inputState](CommandId id, CommandMeta const& meta) {
            if (!commands.condition(id)) {
                return;
            }

            if (!matchCommand(state.input, meta)) {
                return;
            }

            if (!state.activeId) {
                state.activeId = id;
            }

            if (inputState.prevId) {
                if (inputState.wantPrevious && id == state.activeId) {
                    inputState.wantPrevious = false;
                    state.activeId = inputState.prevId;
                }

                if (showPaletteItem(*inputState.prevMeta, inputState.prevId, inputState.prevId == state.activeId)) {
                    commands.invoke(inputState.prevId);
                    ImGui::CloseCurrentPopup();
                    return;
                }
            }

            if (inputState.wantNext && inputState.prevId == state.activeId) {
                inputState.wantNext = false;
                state.activeId = id;
            }

            inputState.prevId = id;
            inputState.prevMeta = &meta;
        });

        if (inputState.prevId &&
            showPaletteItem(*inputState.prevMeta, inputState.prevId, inputState.prevId == state.activeId)) {
            commands.invoke(inputState.prevId);
            ImGui::CloseCurrentPopup();
        }

        if (inputState.wantPrevious) {
            inputState.wantPrevious = false;
            state.activeId = {};
        }

        ImGui::EndTable();
        ImGui::Spacing();

        if (deactivated) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
} // namespace up
