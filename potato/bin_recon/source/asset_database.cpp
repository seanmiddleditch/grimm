// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_database.h"

#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/string_writer.h"

up::AssetDatabase::~AssetDatabase() = default;

auto up::AssetDatabase::pathToUuid(string_view path) noexcept -> UUID {
    for (auto const& [uuid] : _db.query<UUID>("SELECT uuid FROM source_assets WHERE path=?", path)) {
        return uuid;
    }

    return {};
}

auto up::AssetDatabase::uuidToPath(UUID const& uuid) noexcept -> string {
    for (auto const& [filename] : _db.query<zstring_view>("SELECT path FROM source_assets WHERE uuid=?", uuid)) {
        return string{filename};
    }

    return {};
}

auto up::AssetDatabase::findSourceAssetsByFolder(zstring_view folder) -> generator<zstring_view const> {
    for (zstring_view filename : findSourceAssets()) {
        if (path::isParentOf(folder, filename)) {
            co_yield filename;
        }
    }
}

auto up::AssetDatabase::findSourceAssets() -> generator<zstring_view const> {
    for (auto const& [filename] : _db.query<zstring_view>("SELECT path FROM source_assets")) {
        co_yield filename;
    }
}

auto up::AssetDatabase::findSourceAssetsDirtiedBy(zstring_view dependencyPath, uint64 dependencyHash)
    -> generator<zstring_view const> {
    for (auto const& [filename] : _db.query<zstring_view>(
             "SELECT source_assets.path FROM source_assets INNER JOIN import_dependencies ON "
             "source_assets.uuid=import_dependencies.uuid "
             "WHERE import_dependencies.path=? AND import_dependencies.hash<>?",
             dependencyPath,
             dependencyHash)) {
        co_yield filename;
    }
}

auto up::AssetDatabase::findSourceAssetDependencies(UUID const& uuid) -> generator<ImportDependency const> {
    for (auto const& [path, hash] :
         _db.query<zstring_view, uint64>("SELECT path, hash FROM import_dependencies WHERE uuid=?", uuid)) {
        co_yield ImportDependency{.path = string{path}, .contentHash = hash};
    }
}

auto up::AssetDatabase::findImportedAssets(UUID const& uuid) -> generator<ImportedAsset const> {
    for (auto const& [id, name, type, hash] : _db.query<AssetId, zstring_view, zstring_view, uint64>(
             "SELECT id, name, type, hash FROM imported_assets WHERE uuid=?",
             uuid)) {
        co_yield ImportedAsset{.name = name, .type = type, .logicalAssetId = id, .contentHash = hash};
    }
}

auto up::AssetDatabase::createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept -> AssetId {
    uint64 hash = hash_value(uuid);
    if (!logicalName.empty()) {
        hash = hash_combine(hash, hash_value(logicalName));
    }
    return static_cast<AssetId>(hash);
}

void up::AssetDatabase::updateSourceAsset(UUID const& uuid, string_view filename, uint64 sourceHash) {
    [[maybe_unused]] auto const rc = _db.execute(
        "INSERT INTO source_assets "
        "(uuid, path, hash, status) VALUES(?, ?, ?, 'NEW')"
        "ON CONFLICT (uuid) DO UPDATE SET path=excluded.path, "
        "hash=excluded.hash",
        uuid,
        filename,
        sourceHash);
    UP_ASSERT(rc == SqlResult::Ok);
}

bool up::AssetDatabase::isSourceAssetUpToDate(
    UUID const& uuid,
    string_view importerName,
    uint64 importerVersion,
    uint64 sourceHash) {
    auto const [upToDate] = _db.queryOne<bool>(
        "SELECT (hash=? AND importer_name=? AND importer_revision=? AND status='IMPORTED') AS "
        "up_to_date FROM "
        "source_assets WHERE uuid=?",
        sourceHash,
        importerName,
        importerVersion,
        uuid);
    return upToDate;
}

void up::AssetDatabase::beginAssetImport(
    UUID const& uuid,
    string_view importerName,
    string_view assetType,
    uint64 importerVersion) {
    [[maybe_unused]] auto const rc = _db.execute(
        "UPDATE source_assets SET importer_name=?, asset_type=?, importer_revision=?, status='PENDING' WHERE "
        "uuid=?",
        importerName,
        assetType,
        importerVersion,
        uuid);
    UP_ASSERT(rc == SqlResult::Ok);
}

void up::AssetDatabase::finishAssetImport(UUID const& uuid, bool success) {
    [[maybe_unused]] auto rc =
        _db.execute("UPDATE source_assets SET status=? WHERE uuid=?", success ? "IMPORTED"_sv : "FAILED"_sv, uuid);
    UP_ASSERT(rc == SqlResult::Ok);
    (void)_db.execute("DELETE FROM imported_assets WHERE uuid=?", uuid);
    (void)_db.execute("DELETE FROM import_dependencies WHERE uuid=?", uuid);
}

bool up::AssetDatabase::removeSourceAsset(UUID const& uuid) {
    (void)_db.execute("DELETE FROM source_assets WHERE uuid=?", uuid);
    return true;
}

void up::AssetDatabase::addImportDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash) {
    (void)
        _db.execute("INSERT INTO import_dependencies (uuid, path, hash) VALUES(?, ?, ?)", uuid, outputPath, outputHash);
}

void up::AssetDatabase::addAssetImport(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash) {
    (void)_db.execute(
        "INSERT INTO imported_assets (id, uuid, name, type, hash) VALUES(?, ?, ?, ?, ?)",
        createLogicalAssetId(uuid, name).value(),
        uuid,
        name,
        assetType,
        outputHash);
}

bool up::AssetDatabase::open(zstring_view filename) {
    // open the database
    if (_db.open(filename.c_str()) != SqlResult::Ok) {
        return false;
    }

    // ensure the asset database has the correct version
    {
        static constexpr int currentVersion = 18;

        if (_db.execute("CREATE TABLE IF NOT EXISTS version(version INTEGER NOT NULL);") != SqlResult::Ok) {
            return false;
        }
        auto const& [dbVersion] = _db.queryOne<int64>("SELECT version FROM version");
        if (dbVersion != currentVersion) {
            (void)_db.execute("DROP TABLE IF EXISTS source_assets");
            (void)_db.execute("DROP TABLE IF EXISTS import_dependencies");
            (void)_db.execute("DROP TABLE IF EXISTS imported_assets");
            (void)_db.execute("DELETE FROM version");
            auto dbVersionUpdateStmt = _db.prepare("INSERT INTO version (version) VALUES(?)");
            if (dbVersionUpdateStmt.execute(currentVersion) != SqlResult::Ok) {
                return false;
            }
        }
    }

    // ensure the table is created
    if (_db.execute("CREATE TABLE IF NOT EXISTS source_assets "
                    "(uuid TEXT PRIMARY KEY, status TEXT, "
                    "path TEXT, hash INTEGER, asset_type TEXT, "
                    "importer_name TEXT, importer_revision INTEGER)") != SqlResult::Ok) {
        return false;
    }
    if (_db.execute("CREATE TABLE IF NOT EXISTS imported_assets "
                    "(id INT NOT NULL PRIMARY KEY, uuid TEXT, name TEXT, type TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid))") != SqlResult::Ok) {
        return false;
    }
    if (_db.execute("CREATE TABLE IF NOT EXISTS import_dependencies "
                    "(uuid TEXT, path TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid), "
                    "UNIQUE(uuid, path))") != SqlResult::Ok) {
        return false;
    }

    // ensure any left-over source_assets are transitioned to failed status
    (void)_db.execute("UPDATE source_assets SET status='FAILED' WHERE status<>'IMPORTED'");

    return true;
}

bool up::AssetDatabase::close() {
    _db.close();
    return true;
}

void up::AssetDatabase::generateManifest(string_writer& writer) {
    writer.append("# Potato Manifest\n");
    writer.format(".version={}\n", ResourceManifest::version);
    writer.format(
        ":{}|{}|{}|{}|{}|{}\n",
        ResourceManifest::columnUuid,
        ResourceManifest::columnLogicalId,
        ResourceManifest::columnLogicalName,
        ResourceManifest::columnContentType,
        ResourceManifest::columnContentHash,
        ResourceManifest::columnDebugName);

    char fullName[1024];

    for (auto const& [uuid, filename, assetType] :
         _db.query<zstring_view, zstring_view, zstring_view>("SELECT uuid, path, asset_type FROM source_assets")) {
        writer.format("{}|||{}||{}\n", uuid, assetType, filename);
    }

    for (auto const& [uuid, filename] : _db.query<UUID, zstring_view>("SELECT uuid, path FROM source_assets")) {
        for (auto const& [logicalId, logicalName, outputType, outputHash] :
             _db.query<AssetId, zstring_view, zstring_view, uint64>(
                 "SELECT id, name, type, hash FROM imported_assets WHERE uuid=?",
                 uuid)) {
            nanofmt::format_to(fullName, "{}", filename);
            if (!logicalName.empty()) {
                nanofmt::format_append_to(fullName, ":{}", logicalName);
            }

            writer.format(
                "{}|{:016X}|{}|{}|{:016X}|{}\n",
                uuid,
                createLogicalAssetId(uuid, logicalName),
                logicalName,
                outputType,
                outputHash,
                fullName);
        }
    }
}
