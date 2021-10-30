// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_context.h"
#include "importer.h"

#include "potato/reflex/type.h"

up::ImporterContext::ImporterContext(
    UUID const& uuid,
    zstring_view sourceFilePath,
    zstring_view sourceFolderPath,
    zstring_view destinationFolderPath,
    Importer const* importer,
    ImporterConfig const& config,
    vector<string>& dependencies,
    vector<Output>& outputs,
    Logger& logger)
    : _importer(importer)
    , _config(&config)
    , _sourceFilePath(sourceFilePath)
    , _sourceFolderPath(sourceFolderPath)
    , _destinationFolderPath(destinationFolderPath)
    , _uuid(uuid)
    , _sourceDependencies(dependencies)
    , _outputs(outputs)
    , _logger(logger) {}

void up::ImporterContext::addSourceDependency(zstring_view path) {
    _sourceDependencies.push_back(string(path));
}

void up::ImporterContext::addOutput(string logicalAsset, string path, string type) {
    _outputs.push_back(Output{std::move(logicalAsset), std::move(path), std::move(type)});
}

void up::ImporterContext::addMainOutput(string path, string type) {
    _outputs.push_back(Output{{}, std::move(path), std::move(type)});
}

auto up::ImporterContext::config() const noexcept -> up::ImporterConfig const& {
    return *_config;
}
