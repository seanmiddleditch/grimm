// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/asset_loader.h"

#include "potato/runtime/asset.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

#include <Tracy.hpp>

up::AssetLoader::AssetLoader() : _logger("AssetLoader") { }

up::AssetLoader::~AssetLoader() = default;

void up::AssetLoader::bindManifest(box<ResourceManifest> manifest, string casPath) {
    _manifest = std::move(manifest);
    _casPath = std::move(casPath);
    ++_manifestRevision;
}

auto up::AssetLoader::translate(UUID const& uuid, string_view logicalName) const -> AssetId {
    uint64 hash = hash_value(uuid);
    if (!logicalName.empty()) {
        hash = hash_combine(hash, hash_value(logicalName));
    }
    return AssetId{hash};
}

auto up::AssetLoader::debugName(AssetId logicalId) const noexcept -> zstring_view {
    auto const* record = _manifest != nullptr ? _manifest->findRecord(static_cast<uint64>(logicalId)) : nullptr;
    return record != nullptr ? record->filename : zstring_view{};
}

auto up::AssetLoader::loadAssetSync(AssetId id, string_view type) -> UntypedAssetHandle {
    ZoneScopedN("Load Asset Synchronous");

    if (Asset* asset = _findAsset(id); asset != nullptr) {
        return {asset->assetKey(), rc<Asset>{rc_acquire, asset}};
    }

    ResourceManifest::Record const* const record =
        _manifest != nullptr ? _manifest->findRecord(static_cast<uint64>(id)) : nullptr;
    if (record == nullptr) {
        _logger.error("Failed to find asset `{}` ({})", id, type);
        return {};
    }

    if (!type.empty() && record->type != type) {
        _logger.error("Invalid type for asset `{}` [{}] ({}, expected {})", id, record->filename, record->type, type);
        return {};
    }

    AssetLoaderBackend* const backend = _findBackend(record->type);
    if (backend == nullptr) {
        _logger.error("Unknown backend for asset `{}` [{}] ({})", id, record->filename, record->type);
        return {};
    }

    string filename = _makeCasPath(record->hash);

    Stream stream = fs::openRead(filename);
    if (!stream) {
        _logger.error("Unknown asset `{}` [{}] ({}) from `{}`", id, record->filename, record->type, filename);
        return {};
    }

    AssetLoadContext const ctx{.key = {.uuid = record->uuid, .logical = string{}}, .stream = stream, .loader = *this};

    auto asset = backend->loadFromStream(ctx);

    stream.close();

    if (!asset) {
        _logger.error("Load failed for asset `{}` [{}] ({}) from `{}`", id, record->filename, record->type, filename);
        return {};
    }

    _assets.push_back(asset.get());

    return {AssetKey{.uuid = record->uuid, .logical = record->logicalName}, std::move(asset)};
}

void up::AssetLoader::registerBackend(box<AssetLoaderBackend> backend) {
    UP_ASSERT(backend != nullptr);

    _backends.push_back(std::move(backend));
}

auto up::AssetLoader::_findAsset(AssetId id) const noexcept -> Asset* {
    for (Asset* asset : _assets) {
        if (asset->assetId() == id) {
            return asset;
        }
    }
    return nullptr;
}

auto up::AssetLoader::_findBackend(string_view type) const -> AssetLoaderBackend* {
    for (box<AssetLoaderBackend> const& backend : _backends) {
        if (backend->typeName() == type) {
            return backend.get();
        }
    }
    return nullptr;
}

auto up::AssetLoader::_makeCasPath(uint64 contentHash) const -> string {
    char casFilePath[32] = {
        0,
    };
    nanofmt::format_to(
        casFilePath,
        "{:02X}/{:04X}/{:016X}.bin",
        (contentHash >> 56) & 0xFF,
        (contentHash >> 40) & 0XFFFF,
        contentHash);
    return path::join(_casPath, casFilePath);
}

void up::AssetLoader::onAssetReleased(Asset* asset) {
    for (auto it = begin(_assets); it != end(_assets); ++it) {
        if (*it == asset) {
            _assets.erase(it);
            return;
        }
    }
}
