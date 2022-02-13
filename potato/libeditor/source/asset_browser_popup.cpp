// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/asset_browser_popup.h"

#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"

#include <nanofmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

bool up::showAssetBrowser(AssetBrowserState& state, AssetLoader& assetLoader) {
    constexpr auto popupName = "Asset Browser##assetbrowser_popup";

    bool changed = false;

    char filename[64] = {0};

    if (state.wantOpen) {
        state.wantOpen = false;
        if (!ImGui::IsPopupOpen(popupName)) {
            state.searchBuffer[0] = '\0';
            ImGui::OpenPopup(popupName);
        }
    }

    ImVec2 const displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 const minSize{displaySize.x * 0.25f, displaySize.y * 0.25f};
    ImVec2 const maxSize{displaySize.x * 0.5f, displaySize.y * 0.5f};

    ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
    if (ImGui::BeginTitlebarPopup(popupName)) {
        if (ImGui::BeginToolbar("##assetbrowser_toolbar")) {
            if (ImGui::BeginInlineFrame(ICON_FA_SEARCH "##assetbrowser_toolbar_search")) {
                ImGui::InputText("##assetbrowser_search_text", state.searchBuffer, sizeof state.searchBuffer);
                ImGui::EndInlineFrame();
            }
            ImGui::EndToolbar();
        }

        auto const searchLength = stringLength(state.searchBuffer);

        if (ImGui::BeginIconGrid("##assetbrowser_grid")) {
            for (ResourceManifest::Record const& asset : assetLoader.manifest()->records()) {
                if (!state.assetType.empty() && state.assetType != asset.type) {
                    continue;
                }

                nanofmt::format_to(filename, "{}", path::filename(asset.filename));

                if (searchLength > 0 && stringIndexOfNoCase(
                        filename,
                        stringLength(filename),
                        state.searchBuffer, searchLength) < 0) {
                    continue;
                }

                if (ImGui::IconGridItem(static_cast<ImGuiID>(hash_value(asset.uuid)), filename, ICON_FA_FILE)) {
                    state.selected = static_cast<AssetId>(asset.logicalId);
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndIconGrid();
        }

        ImGui::EndPopup();
    }

    return changed;
}
