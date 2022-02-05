// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/command.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    /// @brief Contains the active state for rendering and handling a menu
    class Menu {
        struct MenuDesc;

    public:
        Menu();

        void drawMenu(CommandManager& commands);

    private:
        static constexpr auto noGroup = ~uint32{0};

        struct MenuCategory {
            uint64 hash = 0;
            uint32 groupIndex = 0;
            int priority = 0;
        };

        struct Item {
            uint64 hash = 0;
            CommandId id;
            uint32 stringIndex = 0;
            uint32 childIndex = 0;
            uint32 siblingIndex = 0;
            uint32 groupIndex = 0;
            int priority = 0;
        };

        void _addMenu(MenuDesc&& menu);
        void _rebuild(CommandManager& commands);
        void _drawMenu(uint32 index, uint32 depth, CommandManager& commands);
        bool _hasItems(uint32 index, CommandManager& commands) const;
        auto _findIndexByHash(uint64 hash) const noexcept -> uint32;
        void _insertChild(uint32 parentIndex, uint32 childIndex) noexcept;
        auto _createMenu(string_view menu) -> uint32;
        auto _recordString(string_view string) -> uint32;
        bool _compare(uint32 lhsIndex, uint32 rhsIndex) const noexcept;

        vector<Item> _items;
        vector<MenuCategory> _menus;
        vector<string> _strings;
        uint64 _version = 0;
    };
} // namespace up
