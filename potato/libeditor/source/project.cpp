// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "project.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"

#include <nlohmann/json.hpp>

auto up::Project::loadFromFile(zstring_view filename) -> box<Project> {
    auto const [rs, docText] = fs::readText(filename);
    if (rs != IOResult::Success) {
        return nullptr;
    }

    auto doc = nlohmann::json::parse(docText, nullptr, false);
    if (doc.is_discarded()) {
        return nullptr;
    }

    if (doc["$type"] != "potato.project") {
        return nullptr;
    }

    if (doc["$version"] != 1) {
        return nullptr;
    }

    auto const resourceRoot = doc["resourceRoot"].get<string>();
    auto const projectBasePath = path::parent(filename);

    auto project = new_box<Project>();
    project->_projectFilePath = string(filename);
    project->_resourceRootPath = path::join(projectBasePath, resourceRoot);
    project->_libraryPath = path::join(project->_resourceRootPath, ".library");

    return project;
}
