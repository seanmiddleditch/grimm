// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/menu.h"
#include "ui/action.h"

#include "potato/editor/imgui_ext.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/sequence.h"

#include <imgui.h>

up::shell::Menu::Menu() {
    _strings.push_back("0_default"_s);
}

void up::shell::Menu::bindActions(Actions& actions) {
    _actions = &actions;
}

void up::shell::Menu::addMenu(MenuDesc menu) {
    auto const hash = hash_value(menu.menu);
    auto const groupIndex = _recordString(menu.group);
    _menus.push_back({.hash = hash, .groupIndex = groupIndex, .priority = menu.priority});
    _actionsVersion = 0; // force a rebuild
}

void up::shell::Menu::drawMenu() {
    _rebuild();

    if (ImGui::BeginMainMenuBar()) {
        if (!_items.empty()) {
            _drawMenu(_items.front().childIndex, 0);
        }
        ImGui::EndMainMenuBar();
    }
}

void up::shell::Menu::_rebuild() {
    if (_actions == nullptr || !_actions->refresh(_actionsVersion)) {
        return;
    }

    _items.clear();
    _items.push_back({}); // empty root

    _actions->build([this](auto const id, auto const& action) {
        if (action.menu.empty()) {
            return;
        }

        auto parentIndex = size_t{0};

        auto title = string_view{action.menu};
        auto const hash = hash_value(action.menu);

        auto const groupIndex = _recordString(action.group);

        auto const lastSep = title.find_last_of("\\");
        if (lastSep != zstring_view::npos) {
            auto const menuTitle = title.substr(0, lastSep);
            title = title.substr(lastSep + 1);

            parentIndex = _createMenu(menuTitle);
        }

        auto const titleIndex = _recordString(title);

        auto const newIndex = _items.size();
        _items.push_back(
            {.hash = hash, .id = id, .stringIndex = titleIndex, .groupIndex = groupIndex, .priority = action.priority});
        _insertChild(parentIndex, newIndex);
    });
}

void up::shell::Menu::_drawMenu(size_t index, size_t depth) {
    size_t lastGroup = index != 0 ? _items[index].groupIndex : 0;

    while (index != 0) {
        auto const& item = _items[index];

        if (depth != 0 && item.groupIndex != lastGroup) {
            ImGui::IconMenuSeparator();
            lastGroup = item.groupIndex;
        }

        if (item.id != ActionId::None) {
            auto const checked = _actions->isChecked(item.id);
            auto const enabled = _actions->isEnabled(item.id);
            auto const& actionDesc = _actions->actionAt(item.id);

            if (ImGui::IconMenuItem(
                    _strings[item.stringIndex].c_str(),
                    actionDesc.icon,
                    actionDesc.hotKey.c_str(),
                    checked,
                    enabled)) {
                _actions->invoke(item.id);
            }
        }
        else {
            if (ImGui::BeginIconMenu(_strings[item.stringIndex].c_str())) {
                _drawMenu(item.childIndex, depth + 1);
                ImGui::EndMenu();
            }
        }

        index = item.siblingIndex;
    }
}

auto up::shell::Menu::_findIndexByHash(uint64 hash) const noexcept -> size_t {
    if (hash == 0) {
        return 0;
    }

    for (auto const& [index, item] : enumerate(_items)) {
        if (item.hash == hash) {
            return index;
        }
    }

    return 0;
}

void up::shell::Menu::_insertChild(size_t parentIndex, size_t childIndex) noexcept {
    auto& parent = _items[parentIndex];
    auto& child = _items[childIndex];

    if (parent.childIndex == 0) {
        parent.childIndex = childIndex;
        return;
    }

    if (_compare(childIndex, parent.childIndex)) {
        child.siblingIndex = parent.childIndex;
        parent.childIndex = childIndex;
        return;
    }

    auto index = parent.childIndex;
    while (_items[index].siblingIndex != 0) {
        if (_compare(childIndex, _items[index].siblingIndex)) {
            break;
        }
        index = _items[index].siblingIndex;
    }

    child.siblingIndex = _items[index].siblingIndex;
    _items[index].siblingIndex = childIndex;
}

auto up::shell::Menu::_createMenu(string_view menu) -> size_t {
    if (menu.empty()) {
        return 0;
    }

    auto const hash = hash_value(menu);
    auto const index = _findIndexByHash(hash);
    if (index != 0) {
        return index;
    }

    size_t groupIndex = 0;
    int priority = 0;
    for (auto const& menuGroup : _menus) {
        if (menuGroup.hash == hash) {
            groupIndex = menuGroup.groupIndex;
            priority = menuGroup.priority;
            break;
        }
    }

    auto parentIndex = size_t{0};

    auto const lastSep = menu.find_last_of("\\");
    if (lastSep != zstring_view::npos) {
        parentIndex = _createMenu(menu.substr(0, lastSep));
        menu = menu.substr(lastSep + 1);
    }

    auto const stringIndex = _recordString(menu);
    auto const newIndex = _items.size();
    _items.push_back({.hash = hash, .stringIndex = stringIndex, .groupIndex = groupIndex, .priority = priority});
    _insertChild(parentIndex, newIndex);
    return newIndex;
}

auto up::shell::Menu::_recordString(string_view string) -> size_t {
    for (auto const& [index, str] : enumerate(_strings)) {
        if (string_view{str} == string) {
            return index;
        }
    }

    auto const index = _strings.size();
    _strings.emplace_back(string);
    return index;
}

bool up::shell::Menu::_compare(size_t lhsIndex, size_t rhsIndex) const noexcept {
    auto const& lhsItem = _items[lhsIndex];
    auto const& rhsItem = _items[rhsIndex];

    if (lhsItem.groupIndex != rhsItem.groupIndex) {
        // appears to be a clang-tidy bug?
        //
        // NOLINTNEXTLINE(modernize-use-nullptr)
        return _strings[lhsItem.groupIndex] < _strings[rhsItem.groupIndex];
    }

    if (lhsItem.priority < rhsItem.priority) {
        return true;
    }
    if (lhsItem.priority > rhsItem.priority) {
        return false;
    }

    // appears to be a clang-tidy bug?
    //
    // NOLINTNEXTLINE(modernize-use-nullptr)
    return _strings[lhsItem.stringIndex] < _strings[rhsItem.stringIndex];
}
