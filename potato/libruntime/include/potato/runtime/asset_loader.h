// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "asset.h"
#include "logger.h"
#include "uuid.h"

#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class AssetLoader;
    class ResourceManifest;

    struct AssetLoadContext {
        AssetKey key;
        Stream& stream;
        AssetLoader& loader;
    };

    class AssetLoaderBackend {
    public:
        virtual ~AssetLoaderBackend() = default;

        virtual zstring_view typeName() const noexcept = 0;
        virtual rc<Asset> loadFromStream(AssetLoadContext const& ctx) = 0;
    };

    class AssetLoader {
    public:
        UP_RUNTIME_API AssetLoader();
        UP_RUNTIME_API ~AssetLoader();

        ResourceManifest const* manifest() const noexcept { return _manifest.get(); }
        int manifestRevision() const noexcept { return _manifestRevision; }

        UP_RUNTIME_API void bindManifest(box<ResourceManifest> manifest, string casPath);

        UP_RUNTIME_API AssetId translate(UUID const& uuid, string_view logicalName = {}) const;
        UP_RUNTIME_API zstring_view debugName(AssetId logicalId) const noexcept;

        template <typename AssetT>
        AssetHandle<AssetT> loadAssetSync(AssetId id) {
            zstring_view constexpr typeName = AssetT::assetTypeName;
            return loadAssetSync(id, typeName).cast<AssetT>();
        }
        UP_RUNTIME_API UntypedAssetHandle loadAssetSync(AssetId id, string_view type = {});

        UP_RUNTIME_API void registerBackend(box<AssetLoaderBackend> backend);

        // Note: managing the lifetime of doomed assets should be automatic/internal,
        // based on load requests and such
        UP_RUNTIME_API void collectDoomedAssets();

    private:
        Asset* _findAsset(AssetId id) const noexcept;
        string _makeCasPath(uint64 contentHash) const;
        AssetLoaderBackend* _findBackend(string_view type) const;

        vector<box<AssetLoaderBackend>> _backends;
        vector<Asset*> _assets;
        box<ResourceManifest> _manifest;
        string _casPath;
        Logger _logger;
        int _manifestRevision = 0;
    };
} // namespace up
