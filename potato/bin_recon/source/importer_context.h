// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/uuid.h"
#include "potato/spud/box.h"
#include "potato/spud/concepts.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <string>

namespace up {
    class Logger;
    class Importer;
    struct ImporterConfig;

    class ImporterContext {
    public:
        struct Output {
            string logicalAsset;
            string path;
            string type;
            uint64 contentHash = 0;
        };

        ImporterContext(
            UUID const& uuid,
            zstring_view sourceFilePath,
            zstring_view sourceFolderPath,
            zstring_view destinationFolderPath,
            Importer const* importer,
            ImporterConfig const& config,
            vector<string>& dependencies,
            vector<Output>& outputs,
            Logger& logger);
        ~ImporterContext() = default;

        ImporterContext(ImporterContext&&) = delete;
        ImporterContext& operator=(ImporterContext&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        void addSourceDependency(zstring_view path);
        void addOutput(string logicalAsset, string path, string type);
        void addMainOutput(string path, string type);

        Importer const* importer() const noexcept { return _importer; }
        UUID const& uuid() const noexcept { return _uuid; }
        Logger& logger() noexcept { return _logger; }

        ImporterConfig const& config() const noexcept;
        template <derived_from<ImporterConfig> ImporterConfigT>
        ImporterConfigT const& config() const noexcept {
            return static_cast<ImporterConfigT const&>(config());
        }

    private:
        Importer const* _importer = nullptr;
        ImporterConfig const* _config = nullptr;
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;
        UUID _uuid;

        vector<string>& _sourceDependencies;
        vector<Output>& _outputs;

        Logger& _logger;
    };
} // namespace up
