// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/string.h"

namespace up {
    struct AssetBrowserState {
        bool wantOpen = false;
        AssetId selected;
        string_view assetType;
        char searchBuffer[128] = {0};
    };

    UP_EDITOR_API bool showAssetBrowser(AssetBrowserState& state, AssetLoader& assetLoader);
} // namespace up
