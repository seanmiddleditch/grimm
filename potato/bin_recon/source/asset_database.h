// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "posql.h"

#include "potato/runtime/asset.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/generator.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class ResourceManifest;

    class AssetDatabase {
    public:
        struct ImportDependency {
            zstring_view path;
            uint64 contentHash = 0;
        };

        struct ImportedAsset {
            zstring_view name;
            zstring_view type;
            AssetId logicalAssetId;
            uint64 contentHash = 0;
        };

        // static constexpr zstring_view typeName = "potato.asset.library"_zsv;

        AssetDatabase() = default;
        ~AssetDatabase();

        AssetDatabase(AssetDatabase const&) = delete;
        AssetDatabase& operator=(AssetDatabase const&) = delete;

        auto pathToUuid(string_view path) noexcept -> UUID;
        auto uuidToPath(UUID const& uuid) noexcept -> string;

        static AssetId createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept;

        generator<zstring_view const> findSourceAssetsByFolder(zstring_view folder);
        generator<zstring_view const> findSourceAssets();
        generator<zstring_view const> findSourceAssetsDirtiedBy(zstring_view dependencyPath, uint64 dependencyHash);

        generator<ImportDependency const> findSourceAssetDependencies(UUID const& uuid);
        generator<ImportedAsset const> findImportedAssets(UUID const& uuid);

        void updateSourceAsset(UUID const& uuid, string_view filename, uint64 sourceHash);
        bool removeSourceAsset(UUID const& uuid);

        bool isSourceAssetUpToDate(
            UUID const& uuid,
            string_view importerName,
            uint64 importerVersion,
            uint64 sourceHash);

        void beginAssetImport(
            UUID const& uuid,
            string_view importerName,
            string_view assetType,
            uint64 importerVersion);
        void addImportDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash);
        void addAssetImport(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash);
        void finishAssetImport(UUID const& uuid, bool success);

        bool open(zstring_view filename);
        bool close();

        void generateManifest(string_writer& writer);

        template <callable<posql::Transaction&> Fn>
        void transact(Fn&& fn) {
            posql::Transaction tx = _db.begin();
            fn(tx);
            tx.commit();
        }

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return assetId.value(); }
        };

        Database _db;
    };
} // namespace up
