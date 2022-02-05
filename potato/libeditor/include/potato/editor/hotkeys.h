// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/command.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

namespace up {
    class HotKeys {
    public:
        [[nodiscard]] bool evaluateKey(CommandManager& commands, int keysym, unsigned mods);

    private:
        struct Binding {
            CommandId id;
            int keycode = 0;
            unsigned mods = 0;
        };

        void _update(CommandManager& commands);
        [[nodiscard]] bool _compile(string_view input, int& out_key, unsigned& out_mods) const noexcept;
        [[nodiscard]] auto _stringify(int keycode, unsigned mods) const -> string;

        vector<Binding> _bindings;
        uint64 _version = 0;
    };
} // namespace up
