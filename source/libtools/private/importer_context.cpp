// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_context.h"
#include "importer.h"

#include "potato/reflex/type.h"

up::ImporterContext::ImporterContext(
    zstring_view sourceFilePath,
    zstring_view sourceFolderPath,
    zstring_view destinationFolderPath,
    ImporterConfig const& config,
    Logger& logger)
    : _config(&config)
    , _sourceFilePath(sourceFilePath)
    , _sourceFolderPath(sourceFolderPath)
    , _destinationFolderPath(destinationFolderPath)
    , _logger(logger) {}

up::ImporterContext::~ImporterContext() = default;

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
