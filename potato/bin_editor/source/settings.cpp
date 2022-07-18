// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "settings.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>

bool up::shell::loadShellSettings(zstring_view filename, schema::EditorSettings& settings) {
    if (auto [rs, doc] = readJson(filename); rs == IOResult::Success) {
        return reflex::decodeFromJson(doc, settings);
    }

    return false;
}

bool up::shell::saveShellSettings(zstring_view filename, schema::EditorSettings const& settings) {
    nlohmann::json doc;
    if (!reflex::encodeToJson(doc, settings)) {
        return false;
    }
    auto const rs = fs::writeAllText(filename, doc.dump());
    return rs == IOResult::Success;
}
