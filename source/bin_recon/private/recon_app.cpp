// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"
#include "file_hash_cache.h"
#include "meta_file.h"
#include "recon_log_sink.h"
#include "recon_messages_schema.h"
#include "recon_queue.h"
#include "recon_server.h"

#include "potato/format/format.h"
#include "potato/recon/recon_protocol.h"
#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/overload.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>

up::recon::ReconApp::ReconApp() : _programName("recon"), _logger("recon") {}

up::recon::ReconApp::~ReconApp() = default;

bool up::recon::ReconApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (fs::fileExists(configFile)) {
        _logger.info("Loading config file `{}'", configFile);
        parseConfigFile(_config, configFile, _logger);
    }

    if (!parseArguments(_config, args, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    if (_config.server) {
        Logger::root().attach(new_shared<ReconProtocolLogSink>());
    }

    _project = Project::loadFromFile(_config.project);
    if (_project == nullptr) {
        _logger.error("Failed to load project file `{}`", _config.project);
        return false;
    }

    _manifestPath = path::join(path::Separator::Native, _project->libraryPath(), "manifest.txt");

    if (auto const rs = fs::createDirectories(_project->libraryPath()); rs != IOResult::Success) {
        _logger.error("Failed to create library folder `{}`: {}", _project->libraryPath(), rs);
        return false;
    }

    _temporaryOutputPath = path::join(path::Separator::Native, _project->libraryPath(), "temp");

    _registerImporters();

    auto libraryPath = path::join(path::Separator::Native, _project->libraryPath(), "assets.db");
    if (!_library.open(libraryPath)) {
        _logger.error("Failed to open asset library `{}'", libraryPath);
    }
    _logger.info("Opened asset library `{}'", libraryPath);

    auto hashCachePath = path::join(path::Separator::Native, _project->libraryPath(), "hash_cache.db");
    if (!_hashes.open(hashCachePath)) {
        _logger.error("Failed to open hash cache `{}'", hashCachePath);
    }
    _logger.info("Opened hash cache `{}'", hashCachePath);

    bool success = _config.server ? _runServer() : _runOnce();

    if (!_hashes.close()) {
        _logger.error("Failed to close hash cache `{}'", hashCachePath);
        return false;
    }

    if (!_library.close()) {
        _logger.error("Failed to close library `{}'", libraryPath);
        return false;
    }

    return success;
}

bool up::recon::ReconApp::_runOnce() {
    ReconQueue queue;
    _collectSourceFiles(queue);
    _collectMissingFiles(queue);
    _processQueue(queue);

    if (!_writeManifest()) {
        _logger.error("Failed to write manifest");
        return false;
    }

    return true;
}

bool up::recon::ReconApp::_runServer() {
    ReconQueue queue;

    _collectSourceFiles(queue);
    _collectMissingFiles(queue);

    ReconServer server(_logger);
    server.listenDisconnect([&queue] { queue.enqueTerminate(); });
    server.listenImport([&queue](schema::ReconImportMessage const& msg) { queue.enqueImport(msg.uuid, msg.force); });
    server.listenImportAll([&queue](schema::ReconImportAllMessage const& msg) { queue.enqueImportAll(); });

    // watch the target resource root and auto-convert any items that come in
    auto [rs, watchHandle] = fs::watchDirectory(_project->resourceRootPath(), [&queue](auto const& watch) {
        // ignore dot files (except .meta files)
        if (watch.path != ".meta"_sv && (watch.path.empty() || watch.path.front() == '.')) {
            return;
        }
        switch (watch.action) {
            case fs::WatchAction::Create:
            case fs::WatchAction::Modify:
                queue.enqueImport(string{watch.path});
                break;
            case fs::WatchAction::Delete:
                queue.enqueForget(string{watch.path});
                break;
            case fs::WatchAction::Rename:
                queue.enqueImport(string{watch.path});
                queue.enqueForget(string{watch.renamedFromPath});
                break;
        }
    });
    if (rs != IOResult::Success) {
        return false;
    }

    // handle processing input from the client
    server.start();

    // flush the initially-updated manifest
    server.sendManifest({.path = _manifestPath});

    bool quit = false;
    while (!quit) {
        queue.wait();
        quit = !_processQueue(queue);

        _writeManifest();
        server.sendManifest({.path = _manifestPath});
    }

    watchHandle->close();
    watchHandle = nullptr;

    // server only "fails" if it can't run at all
    return true;
}

bool up::recon::ReconApp::_processQueue(ReconQueue& queue) {
    bool terminate = false;

    ReconQueue::Command cmd;
    while (queue.tryDeque(cmd)) {
        switch (cmd.type) {
            case ReconQueue::Type::Import:
            case ReconQueue::Type::ForceImport:
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                }

                _importFile(cmd.filename, cmd.type == ReconQueue::Type::ForceImport);
                break;
            case ReconQueue::Type::Update:
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                }

                // re-import the file... and forget the file if it's not there
                if (_importFile(cmd.filename) == ReconImportResult::NotFound) {
                    _forgetFile(cmd.filename);
                }
                break;
            case ReconQueue::Type::Forget:
                // if a .meta file is deleted, reimport the source
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                    _importFile(cmd.filename);
                }
                else {
                    _forgetFile(cmd.filename);

                    // if a directory is deleted, we don't get notifications for files under it,
                    // so explicitly delete children
                    vector<string> children;
                    for (auto path : _library.collectAssetPathsByFolder(cmd.filename)) {
                        children.push_back(string{path});
                    }
                    for (string const& path : children) {
                        _forgetFile(path);
                    }
                }
                break;
            case ReconQueue::Type::ImportAll:
            case ReconQueue::Type::ForceImportAll:
                _collectSourceFiles(queue, cmd.type == ReconQueue::Type::ForceImportAll);
                _collectMissingFiles(queue);
                break;
            case ReconQueue::Type::Delete:
                if (auto const* record = _library.findRecordByUuid(cmd.uuid); record != nullptr) {
                    _logger.info("Delete: {}", record->sourcePath);

                    (void)fs::remove(
                        path::join(path::Separator::Native, _project->resourceRootPath(), record->sourcePath));
                    _forgetFile(record->sourcePath);
                }
                break;
            case ReconQueue::Type::Terminate:
                terminate = true;
                break;
        }
    }

    return !terminate;
}

void up::recon::ReconApp::_registerImporters() {
    _importerFactory.registerDefaultImporters();

    for (auto const& mapping : _config.mapping) {
        auto const importer = _importerFactory.findImporterByName(mapping.importer);
        if (importer == nullptr) {
            _logger.error("Unknown importer `{}'", mapping.importer);
        }

        auto config = _importerFactory.parseConfig(*importer, mapping.config);

        string_view pattern = mapping.pattern;
        _importers.push_back(
            {[pattern](string_view filename) { return path::extension(filename) == pattern; },
             importer,
             std::move(config)});
    }
}

auto up::recon::ReconApp::_importFile(zstring_view file, bool force) -> ReconImportResult {
    auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), file.c_str());

    auto const [statRs, stat] = fs::fileStat(osPath);
    if (statRs != IOResult::Success) {
        return ReconImportResult::NotFound;
    }
    bool const isFolder = stat.type == fs::FileType::Directory;

    auto metaPath = _makeMetaFilename(file, isFolder);
    auto const contentHash = isFolder ? 0 : _hashes.hashAssetAtPath(osPath.c_str());

    static const ImporterConfig defaultConfig;

    Mapping const* const mapping = isFolder ? nullptr : _findConverterMapping(file);
    Importer* const importer = mapping != nullptr ? mapping->conveter : nullptr;
    ImporterConfig const& importerConfig = mapping != nullptr ? *mapping->config : defaultConfig;

    ImporterContext
        context(file, _project->resourceRootPath(), _temporaryOutputPath, importer, importerConfig, _logger);
    bool dirty = !_checkMetafile(context, metaPath, true);

    auto const* record = _library.findRecordByUuid(context.uuid());
    dirty |= record == nullptr;
    if (!dirty && importer != nullptr) {
        dirty |=
            record == nullptr || !_isUpToDate(*record, contentHash, *importer) || !_isUpToDate(record->dependencies);
    }

    string_writer importedName;
    {
        if (context.uuid().isValid()) {
            format_append(importedName, "{{{}} ", context.uuid());
        }
        importedName.append(file);
        if (importer != nullptr) {
            format_append(importedName, " ({})", importer->name());
        }
    }

    if (importer == nullptr && !isFolder) {
        _logger.error("{}: unknown file type", importedName);
        return ReconImportResult::UnknownType;
    }

    if (!dirty && !force) {
        _logger.info("{}: up-to-date", importedName);
        return ReconImportResult::UpToDate;
    }

    if (importer != nullptr) {
        _logger.info("{}: importing", importedName);

        if (!importer->import(context)) {
            _logger.error("{}: import failed", importedName);
            return ReconImportResult::Failed;
        }
    }

    AssetDatabase::Imported newRecord;
    newRecord.uuid = context.uuid();
    newRecord.sourcePath = string(file);
    newRecord.sourceContentHash = contentHash;
    if (importer != nullptr) {
        newRecord.importerName = string(importer->name());
        newRecord.importerRevision = importer->revision();
        newRecord.assetType = string(importer->assetType(context));
    }
    else if (isFolder) {
        newRecord.assetType = "potato.folder"_s;
    }

    string_writer logicalAssetName;

    // move outputs to CAS
    //
    for (auto const& output : context.outputs()) {
        auto outputOsPath = path::join(path::Separator::Native, _temporaryOutputPath, output.path);
        auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

        logicalAssetName.clear();
        format_append(
            logicalAssetName,
            "{}{}{}",
            newRecord.sourcePath,
            output.logicalAsset.empty() ? "" : ":",
            output.logicalAsset);
        auto const logicalAssetId = AssetDatabase::createLogicalAssetId(context.uuid(), output.logicalAsset);

        newRecord.outputs.push_back(
            {.name = output.logicalAsset,
             .type = output.type,
             .logicalAssetId = logicalAssetId,
             .contentHash = outputHash});

        fixed_string_writer<32> casPath;
        format_append(
            casPath,
            "{:02X}/{:04X}/{:016X}.bin",
            (outputHash >> 56) & 0xFF,
            (outputHash >> 40) & 0XFFFF,
            outputHash);

        auto casOsPath = path::join(path::Separator::Native, _project->libraryPath(), "cache", casPath);
        auto casOsFolder = string{path::parent(casOsPath)};

        if (auto const rs = fs::createDirectories(casOsFolder); rs != IOResult::Success) {
            _logger.error("Failed to create directory `{}`", casOsFolder);
            continue;
        }

        if (auto const rs = fs::moveFileTo(outputOsPath, casOsPath); rs != IOResult::Success) {
            _logger.error("Failed to move temp file `{}` to CAAS `{}`", outputOsPath, casOsPath);
            continue;
        }
    }

    for (auto const& sourceDepPath : context.sourceDependencies()) {
        auto osPath = path::join(_project->resourceRootPath(), sourceDepPath.c_str());
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        newRecord.dependencies.push_back({string(sourceDepPath), contentHash});
    }

    _library.insertRecord(std::move(newRecord));
    return ReconImportResult::Imported;
}

bool up::recon::ReconApp::_forgetFile(zstring_view file) {
    UUID const uuid = _library.pathToUuid(file);
    if (!uuid.isValid()) {
        return true; // let's not consider this an error; we were asked to forget something we didn't know about
    }

    _logger.info("{}: deleted", file);
    _library.deleteRecordByUuid(uuid);

    // remove the .meta file if it exists;
    // we don't need to do this for a directory, since they could only be deleted
    // if the .meta file is deleted too
    auto metaPath = _makeMetaFilename(file, false);
    auto metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);
    if (fs::fileExists(metaOsPath)) {
        (void)fs::remove(metaOsPath);
    }

    return true;
}

bool up::recon::ReconApp::_isUpToDate(
    AssetDatabase::Imported const& record,
    up::uint64 contentHash,
    Importer const& importer) const noexcept {
    return record.sourceContentHash == contentHash && string_view(record.importerName) == importer.name() &&
        record.importerRevision == importer.revision();
}

bool up::recon::ReconApp::_isUpToDate(span<AssetDatabase::Dependency const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), rec.path.c_str());
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        if (contentHash != rec.contentHash) {
            return false;
        }
    }
    return true;
}

auto up::recon::ReconApp::_findConverterMapping(string_view path) const -> Mapping const* {
    for (auto const& mapping : _importers) {
        if (mapping.predicate(path)) {
            return &mapping;
        }
    }

    return nullptr;
}

auto up::recon::ReconApp::_makeMetaFilename(zstring_view basePath, bool directory) -> string {
    if (directory) {
        return path::join(basePath, ".meta");
    }
    string_writer metaFilePath;
    metaFilePath.append(basePath);
    metaFilePath.append(".meta");
    return metaFilePath.to_string();
}

namespace up::recon {
    static bool loadMetaFile(MetaFile& metaFile, zstring_view osPath) {
        if (Stream stream = fs::openRead(osPath, fs::OpenMode::Text); stream) {
            auto [result, jsonText] = readText(stream);
            if (result == IOResult::Success) {
                if (metaFile.parseJson(jsonText)) {
                    return true;
                }
            }
        }
        return false;
    }
} // namespace up::recon

auto up::recon::ReconApp::_checkMetafile(ImporterContext& ctx, zstring_view metaPath, bool autoCreate) -> bool {
    string metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);

    MetaFile metaFile;
    bool dirty = false;

    if (!loadMetaFile(metaFile, metaOsPath)) {
        metaFile.generate();
        dirty = true;
    }

    if (ctx.importer() != nullptr) {
        if (ctx.importer()->name() != string_view{metaFile.importerName}) {
            metaFile.importerName = string{ctx.importer()->name()};
            dirty = true;
        }

        string_view settings = ctx.importer()->generateSettings(ctx);
        if (settings != metaFile.importerSettings) {
            metaFile.importerSettings = string{settings};
            dirty = true;
        }
    }

    ctx.setUuid(metaFile.uuid);

    if (dirty && autoCreate) {
        _logger.info("Writing meta file `{}'", metaOsPath);

        string jsonText = metaFile.toJson();

        auto stream = fs::openWrite(metaOsPath, fs::OpenMode::Text);
        if (!stream || writeAllText(stream, jsonText) != IOResult::Success) {
            _logger.error("Failed to write meta file for {}", metaOsPath);
            return false;
        }
    }

    // adding meta files to source deps to ensure proper rebuild when meta files change for any reason
    // (like convert settings for a file)
    ctx.addSourceDependency(metaPath);
    return true;
}

void up::recon::ReconApp::_collectSourceFiles(ReconQueue& queue, bool forceUpdate) {
    if (!fs::directoryExists(_project->resourceRootPath())) {
        _logger.error("`{}' does not exist or is not a directory", _project->resourceRootPath());
        return;
    }

    auto cb = [&queue, forceUpdate](auto const& item, int) {
        // do not recurse into the library folder
        //
        if (item.path.starts_with(".library")) {
            return fs::next;
        }

        // skip .meta files
        //
        if (path::extension(item.path) == ".meta") {
            return fs::next;
        }

        queue.enqueImport(string{item.path}, forceUpdate);

        return fs::recurse;
    };

    (void)fs::enumerate(_project->resourceRootPath(), cb);
};

void up::recon::ReconApp::_collectMissingFiles(ReconQueue& queue) {
    for (zstring_view filename : _library.collectAssetPaths()) {
        string osPath = path::join(path::Separator::Native, _project->resourceRootPath(), filename);

        auto const [rs, _] = fs::fileStat(osPath);
        if (rs == IOResult::FileNotFound) {
            queue.enqueForget(string{filename});
        }
    }
}

bool up::recon::ReconApp::_writeManifest() {
    auto manifestFile = fs::openWrite(_manifestPath.c_str(), fs::OpenMode::Text);
    if (!manifestFile) {
        _logger.error("Failed to open manifest `{}'", _manifestPath);
        return false;
    };
    _library.generateManifest(manifestFile);
    manifestFile.flush();
    manifestFile.close();
    return true;
}
