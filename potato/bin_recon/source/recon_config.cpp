// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_config.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/stream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <nlohmann/json.hpp>

bool up::recon::parseArguments(ReconConfig& config, span<char const*> args, Logger& logger) {
    if (args.empty()) {
        return false;
    }

    [[maybe_unused]] auto program = args.front();
    args.pop_front();

    enum {
        ArgNone,
        ArgProject,
        ArgConfig,
    } argMode = ArgNone;

    for (zstring_view arg : args) {
        if (!arg.empty() && arg.front() == '-') {
            if (argMode != ArgNone) {
                logger.error("Unexpected option: {}", arg.c_str());
                return false;
            }

            auto name = arg.substr(1);
            if (name == "project") {
                argMode = ArgProject;
            }
            else if (name == "config") {
                argMode = ArgConfig;
            }
            else if (name == "server") {
                config.server = true;
            }
            else {
                logger.error("Unknown option: {}", arg.c_str());
                return false;
            }
            continue;
        }

        switch (argMode) {
            case ArgNone:
                logger.error("Unexpected value: {}", arg.c_str());
                return false;
            case ArgProject:
                config.project = string(arg);
                argMode = ArgNone;
                break;
            case ArgConfig:
                if (!parseConfigFile(config, arg, logger)) {
                    return false;
                }
                argMode = ArgNone;
                break;
        }
    }

    switch (argMode) {
        case ArgNone:
            return true;
        case ArgProject:
            logger.error("No value provided after `-project' argument");
            return false;
        case ArgConfig:
            logger.error("No value provided after `-config' argument");
            return false;
        default:
            logger.error("No value provided");
            return false;
    }
}

bool up::recon::parseConfigFile(ReconConfig& config, zstring_view path, Logger& logger) {
    auto stream = fs::openRead(path, fs::OpenMode::Text);
    if (!stream) {
        logger.error("Failed to open `{}'", path.c_str());
        return false;
    }

    string text;
    if (readText(stream, text) != IOResult::Success) {
        logger.error("Failed to read `{}'", path.c_str());
        return false;
    }

    return parseConfigString(config, text, path, logger);
}

bool up::recon::parseConfigString(ReconConfig& config, string_view json, zstring_view filename, Logger& logger) {
    auto jsonRoot = nlohmann::json::parse(json.begin(), json.end(), nullptr, false);
    if (!jsonRoot.is_object()) {
        logger.error("Failed to parse file `{}': {}", filename, "unknown parse error");
        return false;
    }

    if (jsonRoot.contains("project") && jsonRoot["project"].is_string()) {
        config.project = jsonRoot["project"].get<string>();
    }

    if (jsonRoot.contains("server") && jsonRoot["server"].is_boolean()) {
        config.server = jsonRoot["server"].get<bool>();
    }

    if (jsonRoot.contains("mapping") && jsonRoot["mapping"].is_array()) {
        for (nlohmann::json const& jsonMapping : jsonRoot["mapping"]) {
            ReconConfigImportMapping& mapping = config.mapping.emplace_back();

            if (jsonMapping.contains("pattern") && jsonMapping["pattern"].is_string()) {
                mapping.pattern = jsonMapping["pattern"].get<string>();
            }

            if (jsonMapping.contains("importer") && jsonMapping["importer"].is_string()) {
                mapping.importer = jsonMapping["importer"].get<string>();
            }

            if (jsonMapping.contains("config")) {
                mapping.config = jsonMapping["config"];
            }
        }
    }

    return true;
}
