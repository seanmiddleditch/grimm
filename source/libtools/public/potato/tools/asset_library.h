// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/format/erased.h"
#include "potato/posql/posql.h"
#include "potato/runtime/asset.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class ResourceManifest;

    class AssetLibrary {
    public:
        struct Dependency {
            string path;
            uint64 contentHash = 0;
        };

        struct Output {
            string name;
            string type;
            AssetId logicalAssetId = AssetId::Invalid;
            uint64 contentHash = 0;
        };

        struct Imported {
            UUID uuid;
            string sourcePath;
            string importerName;
            uint64 importerRevision = 0;
            uint64 sourceContentHash = 0;

            vector<Dependency> dependencies;
            vector<Output> outputs;
        };

        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 13;

        AssetLibrary() = default;
        UP_TOOLS_API ~AssetLibrary();

        AssetLibrary(AssetLibrary const&) = delete;
        AssetLibrary& operator=(AssetLibrary const&) = delete;

        UP_TOOLS_API auto pathToUuid(string_view path) const noexcept -> UUID;
        UP_TOOLS_API auto uuidToPath(UUID const& uuid) const noexcept -> string_view;

        static UP_TOOLS_API AssetId createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept;

        UP_TOOLS_API Imported const* findRecordByUuid(UUID const& uuid) const noexcept;

        UP_TOOLS_API bool insertRecord(Imported record);

        UP_TOOLS_API bool open(zstring_view filename);
        UP_TOOLS_API bool close();

        UP_TOOLS_API void generateManifest(erased_writer writer) const;

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        vector<Imported> _records;
        Database _db;
        Statement _insertAssetStmt;
        Statement _insertOutputStmt;
        Statement _insertDependencyStmt;
        Statement _clearOutputsStmt;
        Statement _clearDependenciesStmt;
    };
} // namespace up
