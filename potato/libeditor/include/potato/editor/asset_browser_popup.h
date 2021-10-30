// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/string.h"

namespace up {
    UP_EDITOR_API bool assetBrowserPopup(
        zstring_view,
        AssetId& inout_asset,
        string_view type,
        AssetLoader& assetLoader);
}
