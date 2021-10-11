// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "meta_file.h"

#include "potato/runtime/json.h"
#include "potato/spud/string_view.h"

#include <nlohmann/json.hpp>

void up::MetaFile::generate() {
    uuid = UUID::generate();
}

auto up::MetaFile::toJson() const -> string {
    nlohmann::json doc;

    doc["$type"] = "potato.asset.meta";
    doc["$version"] = 1;

    doc["id"] = uuid.toString();

    auto importerJson = nlohmann::json();
    if (!importerName.empty()) {
        importerJson["name"] = importerName;
    }
    if (!importerSettings.empty()) {
        importerJson["settings"] = importerSettings;
    }
    doc["importer"] = std::move(importerJson);

    return string(doc.dump(2).c_str());
}

bool up::MetaFile::parseJson(string_view json) {
    if (json.empty()) {
        return false;
    }

    auto doc = nlohmann::json::parse(json.begin(), json.end(), nullptr, false);

    if (!doc.contains("$type")) {
        return false;
    }
    if (auto type = doc["$type"]; !type.is_string() || type != "potato.asset.meta") {
        return false;
    }

    if (!doc.contains("$version")) {
        return false;
    }
    if (auto version = doc["$version"]; !version.is_number_integer() || version != 1) {
        return false;
    }

    if (doc.contains("id")) {
        if (auto const idJson = doc["id"]; idJson.is_string()) {
            uuid = UUID::fromString(idJson.get<string>());
        }
    }

    if (doc.contains("importer")) {
        if (auto const importerJson = doc["importer"]; importerJson.is_object()) {
            if (importerJson.contains("name")) {
                if (auto const importerNameJson = importerJson["name"]; importerNameJson.is_string()) {
                    importerName = importerNameJson.get<string>();
                }
            }
            if (importerJson.contains("settings")) {
                if (auto const importerSettingsJson = importerJson["settings"]; importerSettingsJson.is_string()) {
                    importerSettings = importerSettingsJson.get<string>();
                }
            }
        }
    }

    return uuid.isValid();
}
