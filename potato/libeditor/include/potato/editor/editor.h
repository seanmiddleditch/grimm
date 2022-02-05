// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/editor_common.h"
#include "potato/editor/menu.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class CommandManager;
    class EditorBase;
    class EditorManager;
    class Renderer;

    struct EditorParams {
        EditorId id;
        zstring_view documentPath;
    };

    class EditorFactoryBase {
    public:
        EditorFactoryBase() = default;
        virtual ~EditorFactoryBase() = default;

        EditorFactoryBase(EditorFactoryBase const&) = delete;
        EditorFactoryBase& operator=(EditorFactoryBase const&) = delete;

        virtual EditorTypeId editorType() const noexcept = 0;
        virtual box<EditorBase> createEditor(EditorParams const& params) = 0;
    };

    template <typename EditorT>
    class EditorFactory : public EditorFactoryBase {
    public:
        using EditorType = EditorT;

        EditorTypeId editorType() const noexcept final { return EditorT::editorTypeId; };

        box<EditorBase> createEditor(EditorParams const& params) = 0;
    };

    class EditorBase {
    public:
        using PanelUpdate = delegate<void()>;
        using PanelId = ImGuiID;

        enum class PanelDir { Right, RightLower, Left, LeftLower, Bottom };

        struct Panel {
            string title;
            string imguiLabel;
            bool open = true;
            PanelUpdate update;
            ImGuiID id = 0;
            PanelDir initialDir = PanelDir::Right;
        };

        virtual ~EditorBase() = default;

        EditorBase(EditorBase const&) = delete;
        EditorBase& operator=(EditorBase const&) = delete;

        virtual EditorTypeId editorType() const noexcept = 0;

        /// @brief Display name of the Document, used in menus and window titles.
        /// @return display name.
        virtual zstring_view displayName() const = 0;

        /// @brief Return path to the document this Editor is for, if any.
        virtual zstring_view documentPath() const { return {}; }

        /// @brief Return a globally-unique string that identifies the Editor instance.
        EditorId uniqueId() const noexcept { return _id; }

        /// @brief Updates the UI.
        bool update(CommandManager& commands, float deltaTime);

        ImGuiID imguiWindowId() const noexcept { return _windowId; }
        ImGuiID imguiViewportId() const noexcept;

        /// @brief Renders the ui for the Document.
        virtual void render(Renderer& renderer, float deltaTime) { }

        virtual void tick(float deltaTime) { }

        bool isClosed() const noexcept { return _closed; }
        virtual bool isCloseable() { return true; }
        void close() noexcept { _wantClose = true; }

        void focus() noexcept { _wantFocus = true; }

        void resetLayout() noexcept { _wantReset = true; }

        CommandScope& commandScope() noexcept { return _commands; }

    protected:
        explicit EditorBase(EditorParams const& params);

        ImGuiWindowClass const& panelWindowClass() const noexcept { return _panelClass; }
        ImGuiWindowClass const& contentWindowClass() const noexcept { return _contentClass; }

        void addPanel(string title, PanelDir dir, PanelUpdate update);

        /// @brief Renders the ui for the Document.
        virtual void content(CommandManager&) = 0;
        virtual bool hasMenu() { return false; }
        virtual bool handleClose() { return true; }

    private:
        ImGuiWindowClass _panelClass;
        ImGuiWindowClass _contentClass;

        EditorId _id;
        Menu _menu;
        vector<box<Panel>> _panels;
        CommandScope _commands;
        ImGuiID _windowId;
        bool _wantReset = false;
        bool _wantClose = false;
        bool _wantFocus = false;
        bool _closed = false;
    };

    template <typename EditorT>
    class Editor : public EditorBase {
    public:
        EditorTypeId editorType() const noexcept final { return EditorT::editorTypeId; }

    protected:
        using EditorBase::EditorBase;
    };
} // namespace up
