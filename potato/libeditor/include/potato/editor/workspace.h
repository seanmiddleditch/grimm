// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/command.h"
#include "potato/editor/command_palette.h"
#include "potato/editor/editor.h"
#include "potato/editor/hotkeys.h"
#include "potato/spud/box.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class CommandManager;
    class Renderer;
    class Menu;

    struct EditorFactoryParams {
        Workspace& workspace;
        zstring_view documentPath;
    };

    class Workspace {
    public:
        Workspace();
        ~Workspace();

        static void addCommands(CommandManager& commands);

        void update(Renderer& renderer, CommandManager& commands, float deltaTime);

        void close(EditorId editorId) noexcept;
        void closeAll() noexcept;

        [[nodiscard]] bool isCloseable(EditorId editorId) const noexcept;
        [[nodiscard]] bool isOpen(EditorId editorId) const noexcept;

        [[nodiscard]] EditorId current() const noexcept;
        [[nodiscard]] EditorId focused() const noexcept;

        void showPalette();
        [[nodiscard]] bool evaluateHotkey(CommandManager& commands, int keysym, unsigned mods);

        void resetLayout(EditorId editorId);

        [[nodiscard]] unsigned int imguiFocusedViewport() const noexcept;

        template <typename EditorT, typename... Args>
        EditorId createEditor(Args&&... args);

        EditorId openEditor(EditorTypeId editorType);
        EditorId openEditorForDocument(EditorTypeId editorType, zstring_view documentPath);

        template <derived_from<EditorFactoryBase> EditorFactoryT, typename... Args>
        void addFactory(Args&&... args);

    private:
        struct CloseEditorHandler;
        struct ResetLayoutHandler;

        [[nodiscard]] EditorBase* _getEditor(EditorId editorId) const noexcept;

        vector<box<EditorFactoryBase>> _editorFactories;
        vector<box<EditorBase>> _editors;
        ImGuiWindowClass _documentWindowClass;
        EditorId::underlying_type _nextId{1};
        EditorId _currentEditorId;
        EditorId _focusedEditorId;
        CommandScope _commandScope;
        CommandPaletteState _paletteState;
        HotKeys _hotkeys;
    };

    template <typename EditorT, typename... Args>
    EditorId Workspace::createEditor(Args&&... args) {
        EditorId const id{_nextId++};
        _editors.push_back(new_box<EditorT>(EditorParams{.id = id}, std::forward<Args>(args)...));
        return id;
    }

    template <derived_from<EditorFactoryBase> EditorFactoryT, typename... Args>
    void Workspace::addFactory(Args&&... args) {
        _editorFactories.push_back(new_box<EditorFactoryT>(std::forward<Args>(args)...));
    }

} // namespace up
