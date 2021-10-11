// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "hotkeys.h"

#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_writer.h"

#include <SDL_keycode.h>
#include <imgui.h>

struct KeyMapNames {
    char const* name;
    int keycode;
};
static constexpr KeyMapNames keyMap[] = {
    // Function keys
    {"F1", SDLK_F1},
    {"F2", SDLK_F2},
    {"F3", SDLK_F3},
    {"F4", SDLK_F4},
    {"F5", SDLK_F5},
    {"F6", SDLK_F6},
    {"F7", SDLK_F7},
    {"F8", SDLK_F8},
    {"F9", SDLK_F9},
    {"F10", SDLK_F10},
    // Digits
    {"0", SDLK_0},
    {"1", SDLK_1},
    {"2", SDLK_2},
    {"3", SDLK_3},
    {"4", SDLK_4},
    {"5", SDLK_5},
    {"6", SDLK_6},
    {"7", SDLK_7},
    {"8", SDLK_8},
    {"9", SDLK_9},
    // Letters
    {"A", SDLK_a},
    {"B", SDLK_b},
    {"C", SDLK_c},
    {"D", SDLK_d},
    {"E", SDLK_e},
    {"F", SDLK_f},
    {"G", SDLK_g},
    {"H", SDLK_h},
    {"I", SDLK_i},
    {"J", SDLK_j},
    {"K", SDLK_k},
    {"L", SDLK_l},
    {"M", SDLK_m},
    {"N", SDLK_n},
    {"O", SDLK_o},
    {"P", SDLK_p},
    {"Q", SDLK_q},
    {"R", SDLK_r},
    {"S", SDLK_s},
    {"T", SDLK_t},
    {"U", SDLK_u},
    {"V", SDLK_v},
    {"W", SDLK_w},
    {"X", SDLK_x},
    {"Y", SDLK_y},
    {"Z", SDLK_z},
};
static constexpr size_t keyMapSize = sizeof(keyMap) / sizeof(keyMap[0]);

bool up::editor::HotKeys::evaluateKey(int keysym, unsigned mods, delegate_ref<bool(ActionId)> callback) {
    if (keysym == 0) {
        return false;
    }

    // Normalized mods so left-v-right is erased
    //
    mods &= KMOD_SHIFT | KMOD_CTRL | KMOD_ALT | KMOD_GUI;

    if (0 != (mods & KMOD_SHIFT)) {
        mods |= KMOD_SHIFT;
    }
    if (0 != (mods & KMOD_CTRL)) {
        mods |= KMOD_CTRL;
    }
    if (0 != (mods & KMOD_ALT)) {
        mods |= KMOD_ALT;
    }
    if (0 != (mods & KMOD_GUI)) {
        mods |= KMOD_GUI;
    }

    // Attempt to match the input with registered hot keys
    //
    for (auto& key : _hotKeys) {
        if (key.keycode == keysym && key.mods == mods) {
            if (callback(key.id)) {
                return true;
            }
        }
    }

    return false;
}

void up::editor::HotKeys::clear() {
    _hotKeys.clear();
}

bool up::editor::HotKeys::addHotKey(string_view hotKey, ActionId id) {
    if (hotKey.empty()) {
        return false;
    }

    int keycode = 0;
    unsigned mods = 0;
    if (!_compile(hotKey, keycode, mods)) {
        return false;
    }

    _hotKeys.push_back(HotKey{.id = id, .keycode = keycode, .mods = mods});
    return true;
}

bool up::editor::HotKeys::_compile(string_view input, int& out_key, unsigned& out_mods) const noexcept {
    out_key = 0;
    out_mods = 0;

    while (!input.empty()) {
        if (input.starts_with("Alt+")) {
            out_mods |= KMOD_ALT;
            input = input.substr(4);
            continue;
        }

        if (input.starts_with("Ctrl+")) {
            out_mods |= KMOD_CTRL;
            input = input.substr(5);
            continue;
        }

        if (input.starts_with("Shift+")) {
            out_mods |= KMOD_SHIFT;
            input = input.substr(6);
            continue;
        }

        for (size_t index = 0; index != keyMapSize; ++index) {
            if (keyMap[index].name == input) {
                out_key = keyMap[index].keycode;
                return true;
            }
        }

        return false;
    }

    return false;
}

auto up::editor::HotKeys::_stringify(int keycode, unsigned mods) const -> string {
    string_writer output;

    if (0 != (mods & KMOD_GUI)) {
        output.append("Gui+");
    }
    if (0 != (mods & KMOD_CTRL)) {
        output.append("Ctrl+");
    }
    if (0 != (mods & KMOD_ALT)) {
        output.append("Alt+");
    }
    if (0 != (mods & KMOD_SHIFT)) {
        output.append("Shift+");
    }

    for (size_t index = 0; index != keyMapSize; ++index) {
        if (keyMap[index].keycode == keycode) {
            output.append(keyMap[index].name);
            return std::move(output).to_string();
        }
    }

    output.append("???");

    return std::move(output).to_string();
}
