// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/asset_browser_popup.h"

#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"

#include <nanofmt/format.h>
#include <imgui.h>

bool up::assetBrowserPopup(zstring_view id, AssetId& inout_asset, string_view type, AssetLoader& assetLoader) {
    bool changed = false;

    if (assetLoader.manifest() == nullptr) {
        return false;
    }

    char filename[64] = {
        0,
    };

    ImVec2 const viewportSize = ImGui::GetIO().DisplaySize;
    ImVec2 const minSize{viewportSize.x * 0.25f, viewportSize.y * 0.25f};
    ImVec2 const maxSize{viewportSize.x * 0.5f, viewportSize.y * 0.5f};

    ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
    if (ImGui::BeginPopup(id.c_str())) {
        if (ImGui::BeginIconGrid("##assetbrowser_grid")) {
            for (ResourceManifest::Record const& asset : assetLoader.manifest()->records()) {
                if (!type.empty() && type != asset.type) {
                    continue;
                }

                nanofmt::format_to(filename, "{}", path::filename(asset.filename));

                if (ImGui::IconGridItem(static_cast<ImGuiID>(hash_value(asset.uuid)), filename, ICON_FA_FILE)) {
                    inout_asset = static_cast<AssetId>(asset.logicalId);
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
