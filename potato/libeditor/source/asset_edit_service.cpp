// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_edit_service.h"
#include "icons.h"

#include "potato/runtime/path.h"
#include "potato/spud/hash.h"

namespace up {
    static constexpr AssetEditService::AssetTypeInfo unknownAssetType = {.name = "Unknown"_zsv, .icon = ICON_FA_FILE};
    static constexpr AssetEditService::AssetTypeInfo assetTypes[] = {
        {.name = "Sound"_zsv, .icon = ICON_FA_FILE_AUDIO, .typeHash = hash_value("potato.asset.sound")},
        {.name = "Texture"_zsv, .icon = ICON_FA_FILE_IMAGE, .typeHash = hash_value("potato.asset.texture")},
        {.name = "Shader"_zsv, .icon = ICON_FA_FILE_CODE, .typeHash = hash_value("potato.asset.shader")},
        {.name = "Model"_zsv, .icon = ICON_FA_FILE_ALT, .typeHash = hash_value("potato.asset.model")},
        {.name = "Material"_zsv,
         .extension = ".mat"_zsv,
         .editor = "potato.editor.material"_zsv,
         .icon = unknownAssetType.icon,
         .typeHash = hash_value("potato.asset.material")},
        {.name = "Scene"_zsv,
         .extension = ".scene"_zsv,
         .editor = "potato.editor.scene"_zsv,
         .icon = ICON_FA_FILE_VIDEO,
         .typeHash = hash_value("potato.asset.scene")},
    };
    static constexpr int assetTypeCount = sizeof(assetTypes) / sizeof(assetTypes[0]);
} // namespace up

auto up::AssetEditService::findInfoForAssetTypeHash(uint64 typeHash) const noexcept -> AssetTypeInfo const& {
    for (AssetTypeInfo const& info : assetTypes) {
        if (info.typeHash == typeHash) {
            return info;
        }
    }
    return unknownAssetType;
}

auto up::AssetEditService::findInfoForIndex(int index) const noexcept -> AssetTypeInfo const& {
    if (index >= 0 && index < assetTypeCount) {
        return assetTypes[index];
    }
    return unknownAssetType;
}

auto up::AssetEditService::makeFullPath(zstring_view filename) const -> string {
    return path::normalize(path::join(_assetRoot, filename), path::Separator::Native);
}
