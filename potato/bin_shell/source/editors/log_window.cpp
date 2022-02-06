// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "log_window.h"

#include "potato/editor/editor_manager.h"
#include "potato/editor/imgui_ext.h"
#include "potato/shell/log_history.h"
#include "potato/spud/nameof.h"
#include "potato/spud/utility.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        class LogWindowFactory : public EditorFactory<LogWindow> {
        public:
            explicit LogWindowFactory(LogHistory& history) : _history(history) { }

            box<EditorBase> createEditor(EditorParams const& params) override {
                return new_box<LogWindow>(params, _history);
            }

        private:
            LogHistory& _history;
        };
    } // namespace
} // namespace up::shell

void up::shell::LogWindow::addFactory(EditorManager& editors, LogHistory& history) {
    editors.addFactory<LogWindowFactory>(history);
}

void up::shell::LogWindow::content(CommandManager&) {
    auto severityCombo = [this](LogSeverityMask mask, zstring_view label) {
        bool selected = (mask & _mask) == mask;
        if (ImGui::Selectable(label.c_str(), &selected)) {
            if (ImGui::IsModifierDown(ImGuiKeyModFlags_Shift)) {
                if (selected) {
                    _mask = _mask | mask;
                }
                else {
                    _mask = _mask & ~mask;
                }
            }
            else {
                _mask = mask;
            }
        }
    };

    {
        ImGui::BeginGroup();

        if (ImGui::BeginCombo("Severity", nullptr, ImGuiComboFlags_NoPreview)) {
            severityCombo(LogSeverityMask::Everything, "Everything"_zsv);
            severityCombo(LogSeverityMask::Info, "Info"_zsv);
            severityCombo(LogSeverityMask::Error, "Error"_zsv);
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        ImGui::InputText("Filter", _filter, sizeof(_filter));

        ImGui::SameLine();

        if (ImGui::Button("Test")) {
            Logger::root().info("This is a test");
        }
        ImGui::SameLine();
        if (ImGui::Button("Error")) {
            Logger::root().error("This is an error");
        }

        ImGui::EndGroup();
    }

    ImVec2 const statusSize = ImVec2(
        ImGui::GetContentRegionAvail().x,
        ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y);

    ImGui::BeginChildFrame(
        ImGui::GetID("##frame"),
        ImGui::GetContentRegionAvail() - statusSize,
        ImGuiWindowFlags_AlwaysVerticalScrollbar);

    size_t displayed = 0;

    if (ImGui::BeginTable(
            "##logs",
            4,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_None, 1);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_None, 2);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_None, 6);
        ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_None, 4);
        ImGui::TableHeadersRow();

        for (LogEntry const& log : _history.logs()) {
            if ((to_underlying(toMask(log.severity)) & to_underlying(_mask)) == 0) {
                continue;
            }

            if (_filter[0] != '\0' &&
                stringIndexOfNoCase(log.message.c_str(), log.message.size(), _filter, stringLength(_filter)) == -1) {
                continue;
            }

            ImColor const color = log.severity == LogSeverity::Error ? ImColor(1.f, 0.f, 0.f, 1.f)
                                                                     : ImColor(ImGui::GetColorU32(ImGuiCol_Text));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(color, "%s", toString(log.severity).c_str());
            ImGui::TableNextColumn();
            ImGui::TextColored(color, "%s", log.category.c_str());
            ImGui::TableNextColumn();
            ImGui::TextColored(color, "%s", log.message.c_str());
            if (log.count > 1) {
                ImGui::SameLine();
                ImGui::TextDisabled(" (x%u)", static_cast<unsigned>(log.count));
            }

            ++displayed;
        }

        _stickyBottom = ImGui::IsWindowAppearing() || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY());
        if (_stickyBottom) {
            ImGui::ScrollToItem();
        }

        ImGui::EndTable();
    }

    ImGui::EndChildFrame();

    ImGui::TextDisabled(
        "Showing %u of %u logs",
        static_cast<unsigned>(displayed),
        static_cast<unsigned>(_history.logs().size()));
}
