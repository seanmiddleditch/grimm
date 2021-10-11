// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Project {
    public:
        [[nodiscard]] UP_TOOLS_API static box<Project> loadFromFile(zstring_view filename);

        auto projectFilePath() noexcept -> zstring_view { return _projectFilePath; }
        auto resourceRootPath() noexcept -> zstring_view { return _resourceRootPath; }
        auto libraryPath() noexcept -> zstring_view { return _libraryPath; }

    private:
        string _projectFilePath;
        string _resourceRootPath;
        string _libraryPath;
    };
} // namespace up
