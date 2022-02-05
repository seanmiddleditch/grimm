// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/editor/command.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

struct ImGuiInputTextCallbackData;

namespace up {
    class CommandManager;

    struct CommandPaletteState {
        bool wantOpen = false;
        CommandId activeId;
        char input[128] = {};
    };

    UP_EDITOR_API void showCommandPalette(CommandManager& commands, CommandPaletteState& state);
} // namespace up
