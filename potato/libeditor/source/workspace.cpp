// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/workspace.h"

#include "potato/editor/command.h"
#include "potato/editor/editor.h"
#include "potato/spud/sequence.h"
#include "potato/spud/utility.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace up {
    namespace {
        struct ResetLayoutCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editors.commands.reset_layout"},
                .displayName = "Reset Layout",
                .menu = "View\\Reset Layout::3_panels"};
        };

        struct CloseEditorCommand final : Command {
            static constexpr CommandMeta meta{
                .id = CommandId{"potato.editors.commands.close_document"},
                .displayName = "Close Document",
                .hotkey = "Ctrl+W",
                .menu = "File\\Close Document::7_document"};
        };
    } // namespace

    struct Workspace::ResetLayoutHandler final : CommandHandler<ResetLayoutCommand> {
        ResetLayoutHandler(Workspace& workspace) : _workspace(workspace) { }

        void invoke(ResetLayoutCommand&) override { _workspace.resetLayout(_workspace.focused()); }

    private:
        Workspace& _workspace;
    };

    struct Workspace::CloseEditorHandler final : CommandHandler<CloseEditorCommand> {
        CloseEditorHandler(Workspace& workspace) : _workspace(workspace) { }

        CommandStatus status(CloseEditorCommand const&) override {
            return _workspace.isCloseable(_workspace.focused()) ? CommandStatus::Default : CommandStatus::Disabled;
        }

        void invoke(CloseEditorCommand&) override { _workspace.close(_workspace.focused()); }

    private:
        Workspace& _workspace;
    };

    Workspace::Workspace() {
        _documentWindowClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
        _documentWindowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
        _documentWindowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoSplit;
        _documentWindowClass.TabItemFlagsOverrideSet = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
        _documentWindowClass.DockingAllowUnclassed = false;
        _documentWindowClass.DockingAlwaysTabBar = false;

        _commandScope.addHandler<ResetLayoutHandler>(*this);
        _commandScope.addHandler<CloseEditorHandler>(*this);
    }

    Workspace::~Workspace() = default;

    void Workspace::addCommands(CommandManager& commands) {
        commands.addCommand<ResetLayoutCommand>();
        commands.addCommand<CloseEditorCommand>();
    }

    void Workspace::update(Renderer& renderer, CommandManager& commands, float deltaTime) {
        for (auto it = _editors.begin(); it != _editors.end();) {
            if (it->get()->isClosed()) {
                it = _editors.erase(it);
            }
            else {
                ++it;
            }
        }

        for (auto index : sequence(_editors.size())) {
            _editors[index]->tick(deltaTime);
        }

        auto const dockspaceId =
            ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_NoSplit, &_documentWindowClass);

        for (auto index : sequence(_editors.size())) {
            EditorBase* editor = _editors[index].get();

            if (editor->isClosed()) {
                continue;
            }

            commands.pushScope(_commandScope);
            commands.pushScope(editor->commandScope());

            editor->render(renderer, deltaTime);

            _currentEditorId = editor->uniqueId();

            ImGui::SetNextWindowDockID(dockspaceId, editor->isCloseable() ? ImGuiCond_FirstUseEver : ImGuiCond_Always);
            ImGui::SetNextWindowClass(&_documentWindowClass);
            if (!_focusedEditorId) {
                ImGui::SetNextWindowFocus();
            }
            if (editor->update(commands, deltaTime)) {
                _focusedEditorId = editor->uniqueId();
                showCommandPalette(commands, _paletteState);
            }
            ImGui::End();

            commands.popScope(editor->commandScope());
            commands.popScope(_commandScope);

            _currentEditorId = EditorId{};
        }
    }

    void Workspace::close(EditorId editorId) noexcept {
        if (EditorBase* const editor = _getEditor(editorId); editor != nullptr) {
            editor->close();
        }
    }

    void Workspace::closeAll() noexcept {
        for (auto const& editor : _editors) {
            if (editor->isCloseable()) {
                editor->close();
            }
        }
    }

    bool Workspace::isCloseable(EditorId editorId) const noexcept {
        if (EditorBase* const editor = _getEditor(editorId); editor != nullptr) {
            return editor->isCloseable();
        }
        return false;
    }

    bool Workspace::isOpen(EditorId editorId) const noexcept {
        if (EditorBase* const editor = _getEditor(editorId); editor != nullptr) {
            return !editor->isClosed();
        }
        return false;
    }

    EditorId Workspace::current() const noexcept { return _currentEditorId; }

    EditorId Workspace::focused() const noexcept { return _focusedEditorId; }

    void Workspace::showPalette() { _paletteState.wantOpen = true; }

    bool Workspace::evaluateHotkey(CommandManager& commands, int keysym, unsigned mods) {
        EditorBase* const editor = _getEditor(focused());
        if (editor != nullptr) {
            commands.pushScope(editor->commandScope());
        }
        bool const handled = _hotkeys.evaluateKey(commands, keysym, mods);
        if (editor != nullptr) {
            commands.popScope(editor->commandScope());
        }
        return handled;
    }

    void Workspace::resetLayout(EditorId editorId) {
        if (EditorBase* const editor = _getEditor(editorId); editor != nullptr) {
            editor->resetLayout();
        }
    }

    unsigned int Workspace::imguiFocusedViewport() const noexcept {
        EditorBase const* const editor = _getEditor(focused());
        return editor != nullptr ? editor->imguiViewportId() : 0;
    }

    EditorBase* Workspace::_getEditor(EditorId editorId) const noexcept {
        for (auto const& editor : _editors) {
            if (editor->uniqueId() == editorId) {
                return editor.get();
            }
        }
        return nullptr;
    }

    EditorId Workspace::openEditor(EditorTypeId editorType) {
        for (auto const& editor : _editors) {
            if (editor->editorType() == editorType) {
                editor->focus();
                return editor->uniqueId();
            }
        }

        for (auto const& factory : _editorFactories) {
            if (factory->editorType() == editorType) {
                EditorId const id(_nextId++);
                auto editor = factory->createEditor({.id = id});
                if (editor != nullptr) {
                    UP_ASSERT(editor->editorType() == editorType);
                    _editors.push_back(std::move(editor));
                    return id;
                }
                break;
            }
        }

        return {};
    }

    EditorId Workspace::openEditorForDocument(EditorTypeId editorType, zstring_view documentPath) {
        for (auto const& editor : _editors) {
            if (editor->editorType() == editorType && editor->documentPath() == documentPath) {
                editor->focus();
                return editor->uniqueId();
            }
        }

        for (auto const& factory : _editorFactories) {
            if (factory->editorType() == editorType) {
                EditorId const id(_nextId++);
                auto editor = factory->createEditor({.id = id, .documentPath = documentPath});
                if (editor != nullptr) {
                    UP_ASSERT(editor->editorType() == editorType);
                    UP_ASSERT(editor->documentPath() == documentPath);
                    _editors.push_back(std::move(editor));
                    return id;
                }
                break;
            }
        }

        return {};
    }
} // namespace up
