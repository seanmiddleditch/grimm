// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/shell/editor.h"

#include "potato/spud/sequence.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/utility.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::Editor::Editor(zstring_view className) {
    _panelClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _panelClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoTaskBarIcon;
    _panelClass.TabItemFlagsOverrideSet = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
    _panelClass.DockingAllowUnclassed = false;
    _panelClass.DockingAlwaysTabBar = false;

    _contentClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _contentClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoDockingOverMe |
        ImGuiDockNodeFlags_NoDockingSplitMe | ImGuiDockNodeFlags_NoTabBar;
    _contentClass.TabItemFlagsOverrideSet = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
    _contentClass.DockingAllowUnclassed = false;
    _contentClass.DockingAlwaysTabBar = false;

    _actions.addAction({.menu = "View\\Reset Layout", .group = "3_panels"_s, .action = [this] {
                            this->resetLayout();
                        }});
}

bool up::shell::Editor::updateUi() {
    char editorTitle[128];
    char contentTitle[128];

    nanofmt::format_to(editorTitle, "{}##{}", displayName(), this);
    nanofmt::format_to(contentTitle, "Document##{}", this);

    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    bool isEditorOpen = false;
    if (isClosable()) {
        bool wantOpen = true;
        isEditorOpen = ImGui::Begin(editorTitle, &wantOpen, windowFlags);
        _wantClose = _wantClose || !wantOpen;
    }
    else {
        isEditorOpen = ImGui::Begin(editorTitle, nullptr, windowFlags);
    }
    ImGui::PopStyleVar(1);

    auto const dockSpaceId = ImGui::GetID("DockSpace");
    if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr || _wantReset) {
        _wantReset = false;

        ImGui::DockBuilderRemoveNode(dockSpaceId);
        auto mainId = ImGui::DockBuilderAddNode(
            dockSpaceId,
            ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoWindowMenuButton);
        ImGui::DockBuilderSetNodeSize(mainId, ImGui::GetWindowSize());

        bool hasRight = false;
        bool hasRightLower = false;
        bool hasLeft = false;
        bool hasLeftLower = false;

        for (auto const& panel : _panels) {
            hasRight |= panel->initialDir == PanelDir::Right;
            hasRightLower |= panel->initialDir == PanelDir::RightLower;
            hasLeft |= panel->initialDir == PanelDir::Left;
            hasLeftLower |= panel->initialDir == PanelDir::LeftLower;
        }

        ImGuiID right = 0;
        ImGuiID rightLower = 0;
        ImGuiID left = 0;
        ImGuiID leftLower = 0;

        if (hasRight) {
            right = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.25f, nullptr, &mainId);
            if (hasRightLower) {
                rightLower = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.5f, nullptr, &right);
            }
        }
        else if (hasRightLower) {
            right = rightLower = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.25f, nullptr, &mainId);
        }

        if (hasLeft) {
            left = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Left, 0.25f, nullptr, &mainId);
            if (hasLeftLower) {
                leftLower = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.6f, nullptr, &left);
            }
        }
        else if (hasLeftLower) {
            left = leftLower = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Left, 0.25f, nullptr, &mainId);
        }

        for (auto const& panel : _panels) {
            if (panel->initialDir == PanelDir::Right) {
                ImGui::DockBuilderDockWindow(panel->imguiLabel.c_str(), right);
            }
            else if (panel->initialDir == PanelDir::RightLower) {
                ImGui::DockBuilderDockWindow(panel->imguiLabel.c_str(), rightLower);
            }
            else if (panel->initialDir == PanelDir::Left) {
                ImGui::DockBuilderDockWindow(panel->imguiLabel.c_str(), left);
            }
            else if (panel->initialDir == PanelDir::LeftLower) {
                ImGui::DockBuilderDockWindow(panel->imguiLabel.c_str(), leftLower);
            }
        }

        ImGui::DockBuilderDockWindow(contentTitle, mainId);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(
        dockSpaceId,
        {},
        isEditorOpen ? ImGuiDockNodeFlags_None : ImGuiDockNodeFlags_KeepAliveOnly,
        &_panelClass);

    if (isEditorOpen) {
        ImGui::SetNextWindowClass(&_contentClass);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        auto const contentOpen = ImGui::Begin(contentTitle, nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::PopStyleVar(1);

        if (contentOpen) {
            content();
        }

        ImGui::End();

        for (auto const& panel : _panels) {
            if (panel->open) {
                ImGui::SetNextWindowClass(&_panelClass);
                if (ImGui::Begin(panel->imguiLabel.c_str(), &panel->open, ImGuiWindowFlags_NoCollapse)) {
                    panel->update();
                }
                ImGui::End();
            }
        }
    }

    if (_wantClose && !_closed) {
        _closed = handleClose();
    }

    auto const active = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ImGui::End();

    return active;
}

void up::shell::Editor::addPanel(string title, PanelDir dir, PanelUpdate update) {
    UP_GUARD_VOID(!title.empty());
    UP_GUARD_VOID(update != nullptr);

    auto panel = new_box<Panel>();
    panel->title = std::move(title);
    panel->update = std::move(update);
    panel->initialDir = dir;

    char tmp[128] = {};
    nanofmt::format_to(tmp, "{}##{}", panel->title, panel.get());
    panel->imguiLabel = tmp;

    panel->id = ImGui::GetID(panel.get());

    nanofmt::format_to(tmp, "View\\Panels\\{}", panel->title);

    _actions.addAction(
        {.menu = tmp,
         .group = "3_panels"_s,
         .checked = [ptr = panel.get()] { return ptr->open; },
         .action =
             [ptr = panel.get()] {
                 ptr->open = !ptr->open;
             }});

    _panels.push_back(std::move(panel));
}

void up::shell::Editor::activate(bool active, Actions& actions) {
    if (active == _active) {
        return;
    }

    _active = active;

    if (active) {
        actions.addGroup(&_actions);
    }
    else {
        actions.removeGroup(&_actions);
    }
}
