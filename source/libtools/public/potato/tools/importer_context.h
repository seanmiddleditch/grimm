// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "asset_record.h"

#include "potato/spud/std_iostream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <string>

namespace up {
    class FileSystem;
    class Logger;
} // namespace up

namespace up {
    class ImporterContext {
    public:
        struct Output {
            string_view logicalAsset;
            string path;
        };

        ImporterContext(zstring_view sourceFilePath,
            zstring_view sourceFolderPath,
            zstring_view destinationFolderPath,
            FileSystem& fileSystem,
            Logger& logger)
            : _sourceFilePath(sourceFilePath)
            , _sourceFolderPath(sourceFolderPath)
            , _destinationFolderPath(destinationFolderPath)
            , _fileSystem(fileSystem)
            , _logger(logger) {}

        ImporterContext(ImporterContext&&) = delete;
        ImporterContext& operator=(ImporterContext&&) = delete;

        auto sourceFilePath() const noexcept { return _sourceFilePath; }
        auto sourceFolderPath() const noexcept { return _sourceFolderPath; }
        auto destinationFolderPath() const noexcept { return _destinationFolderPath; }

        UP_TOOLS_API void addLogicalAsset(string name);
        UP_TOOLS_API void addSourceDependency(zstring_view path);
        void addOutput(string_view logicalAsset, zstring_view path);
        void addMainOutput(zstring_view path);

        view<string> sourceDependencies() const noexcept { return _sourceDependencies; }
        view<string> logicalAssets() const noexcept { return _logicalAssets; }
        view<Output> outputs() const noexcept { return _outputs; }

        FileSystem& fileSystem() noexcept { return _fileSystem; }
        Logger& logger() noexcept { return _logger; }

    private:
        zstring_view _sourceFilePath;
        zstring_view _sourceFolderPath;
        zstring_view _destinationFolderPath;

        vector<string> _sourceDependencies;
        vector<string> _logicalAssets;
        vector<Output> _outputs;

        FileSystem& _fileSystem;
        Logger& _logger;
    };
} // namespace up
