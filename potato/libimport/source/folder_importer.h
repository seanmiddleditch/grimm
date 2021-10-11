// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class FolderImporter : public Importer {
    public:
        bool import(ImporterContext& ctx) override { return true; }
        string_view assetType(ImporterContext& ctx) const noexcept override { return "potato.folder"_sv; }

        string_view name() const noexcept override { return "folder"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
