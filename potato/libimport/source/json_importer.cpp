// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "json_importer.h"
#include "importer_configs_schema.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/spud/std_iostream.h"

#include <nlohmann/json.hpp>
#include <fstream>

up::JsonImporter::JsonImporter() = default;

up::JsonImporter::~JsonImporter() = default;

auto up::JsonImporter::configType() const noexcept -> reflex::TypeInfo const& {
    return reflex::getTypeInfo<JsonImporterConfig>();
}

bool up::JsonImporter::import(ImporterContext& ctx) {
    auto const& config = ctx.config<JsonImporterConfig>();

    auto sourceAbsolutePath = path::join(ctx.sourceFolderPath(), ctx.sourceFilePath());
    auto destAbsolutePath = path::join(ctx.destinationFolderPath(), ctx.sourceFilePath());

    string destParentAbsolutePath(path::parent(destAbsolutePath));

    if (!fs::directoryExists(destParentAbsolutePath.c_str())) {
        if (fs::createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ifstream inFile(sourceAbsolutePath.c_str());
    if (!inFile) {
        ctx.logger().error("Failed to open `{}'", sourceAbsolutePath);
        return false;
    }

    nlohmann::json doc = nlohmann::json::parse(inFile, nullptr, false);
    if (doc.is_discarded()) {
        ctx.logger().error("Failed to parse `{}': {}", sourceAbsolutePath, "unknown parse error");
        return false;
    }

    inFile.close();

    std::ofstream outFile(destAbsolutePath.c_str());
    if (!outFile) {
        ctx.logger().error("Failed to open `{}'", destAbsolutePath);
        return false;
    }

    outFile << doc;

    if (!outFile) {
        ctx.logger().error("Failed to write to `{}'", destAbsolutePath);
        return false;
    }

    outFile.close();

    // output has same name as input
    ctx.addMainOutput(string{ctx.sourceFilePath()}, config.type);

    ctx.logger().info("Minified `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}

auto up::JsonImporter::assetType(ImporterContext& ctx) const noexcept -> string_view {
    auto const& config = ctx.config<JsonImporterConfig>();
    return config.type;
}
