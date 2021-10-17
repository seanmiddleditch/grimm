// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor_group.h"

#include "potato/shell/editor.h"
#include "potato/spud/sequence.h"
#include "potato/spud/utility.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::EditorGroup::EditorGroup(Actions& actions) : _actions(actions) {
    _documentWindowClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _documentWindowClass.DockingAllowUnclassed = false;
    _documentWindowClass.DockingAlwaysTabBar = true;
}

up::shell::EditorGroup::~EditorGroup() = default;

void up::shell::EditorGroup::update(Renderer& renderer, float deltaTime) {
    for (auto it = _editors.begin(); it != _editors.end();) {
        if (it->get()->isClosed()) {
            if (it->get() == _active) {
                _setActive(nullptr);
            }
            it = _editors.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto index : sequence(_editors.size())) {
        _editors[index]->tick(deltaTime);
    }

    auto const dockspaceId = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspaceId, {}, ImGuiDockNodeFlags_NoWindowMenuButton, &_documentWindowClass);

    for (auto index : sequence(_editors.size())) {
        Editor* editor = _editors[index].get();

        if (editor->isClosed()) {
            continue;
        }

        editor->render(renderer, deltaTime);

        ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_documentWindowClass);
        if (_active == nullptr) {
            ImGui::SetNextWindowFocus();
        }
        auto const active = editor->updateUi();

        if (!editor->isClosed() && active) {
            _setActive(editor);
        }
    }
}

void up::shell::EditorGroup::closeAll() noexcept {
    for (auto const& editor : _editors) {
        editor->close();
    }
}

void up::shell::EditorGroup::closeActive() noexcept {
    if (_active != nullptr) {
        _active->close();
    }
}

bool up::shell::EditorGroup::canCloseActive() const noexcept {
    return _active != nullptr && _active->isClosable();
}

void up::shell::EditorGroup::open(box<Editor> editor) {
    _editors.push_back(std::move(editor));
}

void up::shell::EditorGroup::_setActive(Editor* editor) {
    if (editor == _active) {
        return;
    }

    if (_active != nullptr) {
        _active->activate(false, _actions);
    }

    _active = editor;

    if (_active != nullptr) {
        _active->activate(true, _actions);
    }
}
