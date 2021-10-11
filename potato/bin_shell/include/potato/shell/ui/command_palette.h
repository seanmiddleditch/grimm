// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "ui/action.h"

#include "potato/spud/string.h"
#include "potato/spud/vector.h"

struct ImGuiInputTextCallbackData;

namespace up::shell {
    /// @brief Handles the command palette UI
    class CommandPalette {
    public:
        void bindActions(Actions& actions);

        void show();
        void close();

        void drawPalette();

    private:
        struct Command {
            ActionId id = ActionId::None;
        };

        static int _callbackWrapper(ImGuiInputTextCallbackData* data);
        void _rebuild();
        int _callback(ImGuiInputTextCallbackData* data);
        bool _execute() const;
        void _updateMatches();

        size_t _activeIndex = 0;
        char _input[128] = {0};
        vector<Command> _commands;
        vector<size_t> _matches;
        ActionGroup _commandActions;
        Actions* _actions = nullptr;
        uint64 _actionsVersion = 0;
        bool _open = false;
    };
} // namespace up::shell
