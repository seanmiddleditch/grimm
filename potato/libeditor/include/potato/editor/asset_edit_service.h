// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class AssetEditService {
    public:
        struct AssetTypeInfo {
            zstring_view name;
            zstring_view extension;
            zstring_view editor;
            char8_t const* icon = nullptr;
            uint64 typeHash = 0;
        };

        void setAssetRoot(string folder) noexcept { _assetRoot = std::move(folder); }

        UP_EDITOR_API AssetTypeInfo const& findInfoForAssetTypeHash(uint64 typeHash) const noexcept;
        UP_EDITOR_API AssetTypeInfo const& findInfoForIndex(int index) const noexcept;
        UP_EDITOR_API string makeFullPath(zstring_view filename) const;

    private:
        string _assetRoot;
    };
} // namespace up
