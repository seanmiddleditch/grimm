// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate_ref.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

namespace up {
    enum class ActionId : uint64;
}

namespace up::inline editor {
    class HotKeys {
    public:
        [[nodiscard]] bool evaluateKey(int keysym, unsigned mods, delegate_ref<bool(ActionId)> callback);

        void clear();
        bool addHotKey(string_view hotKey, ActionId id);

    private:
        struct HotKey {
            ActionId id{};
            int keycode = 0;
            unsigned mods = 0;
        };

        [[nodiscard]] bool _compile(string_view input, int& out_key, unsigned& out_mods) const noexcept;
        [[nodiscard]] auto _stringify(int keycode, unsigned mods) const -> string;

        vector<HotKey> _hotKeys;
    };
} // namespace up::inline editor
