// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/menu.h"

#include "potato/editor/command.h"
#include "potato/editor/imgui_command.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/sequence.h"

#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>

namespace up {
    struct Menu::MenuDesc {
        string_view menu;
        string_view group = "0_default"_sv;
        int priority = 1000;
    };

    Menu::Menu() {
        _strings.push_back("0_default"_s);

        _addMenu({.menu = "File"_sv, .group = "1_file"_sv});
        _addMenu({.menu = "File\\New"_sv, .group = "2_new"_sv});
        _addMenu({.menu = "File\\Settings"_sv, .group = "9_settings"_sv});
        _addMenu({.menu = "Edit"_sv, .group = "3_edit"_sv});
        _addMenu({.menu = "View"_sv, .group = "5_view"_sv});
        _addMenu({.menu = "View\\Options"_sv, .group = "5_options"_sv});
        _addMenu({.menu = "View\\Panels"_sv, .group = "3_panels"_sv});
        _addMenu({.menu = "Actions"_sv, .group = "7_actions"_sv});
        _addMenu({.menu = "Help"_sv, .group = "9_help"_sv});
    }

    void Menu::drawMenu(CommandManager& commands) {
        _rebuild(commands);

        if (ImGui::BeginMenuBar()) {
            if (!_items.empty()) {
                _drawMenu(_items.front().childIndex, 0, commands);
            }
            ImGui::EndMenuBar();
        }
    }

    void Menu::_addMenu(MenuDesc&& menu) {
        auto const hash = hash_value(menu.menu);
        auto const groupIndex = _recordString(menu.group);
        _menus.push_back({.hash = hash, .groupIndex = groupIndex, .priority = menu.priority});
        _version = 0; // force a rebuild
    }

    void Menu::_rebuild(CommandManager& commands) {
        bool dirty = false;

        if (commands.refresh(_version)) {
            dirty = true;
        }

        if (!dirty) {
            return;
        }

        _items.clear();
        _items.push_back({}); // empty root

        commands.each([this](auto const id, auto const& command) {
            if (command.menu == nullptr) {
                return;
            }

            uint32 parentIndex{0};
            uint32 groupIndex{0};

            string_view title{command.menu};
            auto const hash = hash_value(command.menu);

            string_view parent;
            string_view group;
            int priority{1000};

            auto const menuSep = title.find_last_of("\\");
            if (menuSep != string_view::npos) {
                parent = title.substr(0, menuSep);
                title = title.substr(menuSep + 1);
            }

            auto const groupSep = title.find_first_of("::");
            if (groupSep != string_view::npos) {
                group = title.substr(groupSep + 2);
                title = title.substr(0, groupSep);
            }

            auto const prioritySep = group.find_first_of("::");
            if (prioritySep != string_view::npos) {
                auto const priorityStr = group.substr(prioritySep + 2);
                group = group.substr(0, prioritySep);

                std::from_chars(priorityStr.data(), priorityStr.data() + priorityStr.size(), priority);
            }

            if (!group.empty()) {
                groupIndex = _recordString(group);
            }
            if (!parent.empty()) {
                parentIndex = _createMenu(parent);
            }

            auto const titleIndex = _recordString(title);

            auto const newIndex = _items.size();
            _items.push_back(
                {.hash = hash,
                 .id = id,
                 .stringIndex = static_cast<uint32>(titleIndex),
                 .groupIndex = static_cast<uint32>(groupIndex),
                 .priority = priority});
            _insertChild(parentIndex, static_cast<uint32>(newIndex));
        });
    }

    void Menu::_drawMenu(uint32 index, uint32 depth, CommandManager& commands) {
        uint32 lastGroup = index != 0 ? _items[index].groupIndex : 0;

        while (index != 0) {
            auto const& item = _items[index];

            if (depth != 0 && item.groupIndex != lastGroup) {
                ImGui::Separator();
                lastGroup = item.groupIndex;
            }

            if (item.id.valid()) {
                if (!commands.condition(item.id)) {
                    index = item.siblingIndex;
                    continue;
                }

                if (ImGui::CommandMenuItem(commands, item.id)) {
                    commands.invoke(item.id);
                }
            }
            else if (_hasItems(item.childIndex, commands)) {
                if (ImGui::BeginMenu(_strings[item.stringIndex].c_str())) {
                    _drawMenu(item.childIndex, depth + 1, commands);
                    ImGui::EndMenu();
                }
            }

            index = item.siblingIndex;
        }
    }

    bool Menu::_hasItems(uint32 index, CommandManager& commands) const {
        while (index != 0) {
            auto const& item = _items[index];
            if (item.id.valid()) {
                if (commands.condition(item.id)) {
                    return true;
                }
            }
            else if (_hasItems(item.childIndex, commands)) {
                return true;
            }
            index = item.siblingIndex;
        }
        return false;
    }

    auto Menu::_findIndexByHash(uint64 hash) const noexcept -> uint32 {
        if (hash == 0) {
            return 0;
        }

        for (auto const& [index, item] : enumerate(_items)) {
            if (item.hash == hash) {
                return static_cast<uint32>(index);
            }
        }

        return 0;
    }

    void Menu::_insertChild(uint32 parentIndex, uint32 childIndex) noexcept {
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

    auto Menu::_createMenu(string_view menu) -> uint32 {
        if (menu.empty()) {
            return 0;
        }

        auto const hash = hash_value(menu);
        auto const index = _findIndexByHash(hash);
        if (index != 0) {
            return index;
        }

        uint32 groupIndex = 0;
        int priority = 0;
        for (auto const& menuGroup : _menus) {
            if (menuGroup.hash == hash) {
                groupIndex = menuGroup.groupIndex;
                priority = menuGroup.priority;
                break;
            }
        }

        auto parentIndex = uint32{0};

        auto const lastSep = menu.find_last_of("\\");
        if (lastSep != zstring_view::npos) {
            parentIndex = _createMenu(menu.substr(0, lastSep));
            menu = menu.substr(lastSep + 1);
        }

        auto const stringIndex = _recordString(menu);
        auto const newIndex = _items.size();
        _items.push_back({.hash = hash, .stringIndex = stringIndex, .groupIndex = groupIndex, .priority = priority});
        _insertChild(parentIndex, static_cast<uint32>(newIndex));
        return static_cast<uint32>(newIndex);
    }

    auto Menu::_recordString(string_view string) -> uint32 {
        for (auto const& [index, str] : enumerate(_strings)) {
            if (string_view{str} == string) {
                return static_cast<uint32>(index);
            }
        }

        auto const index = _strings.size();
        _strings.emplace_back(string);
        return static_cast<uint32>(index);
    }

    bool Menu::_compare(uint32 lhsIndex, uint32 rhsIndex) const noexcept {
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

} // namespace up
