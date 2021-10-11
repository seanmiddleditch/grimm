// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class Importer;
    struct ImporterConfig;

    class ImporterFactory {
    public:
        UP_IMPORT_API ImporterFactory();
        UP_IMPORT_API ~ImporterFactory();

        ImporterFactory(ImporterFactory const&) = delete;
        ImporterFactory& operator=(ImporterFactory const&) = delete;

        UP_IMPORT_API Importer* findImporterByName(string_view name) const noexcept;

        UP_IMPORT_API void registerImporter(box<Importer> importer);

        UP_IMPORT_API void registerDefaultImporters();

        UP_IMPORT_API box<ImporterConfig> parseConfig(Importer const& importer, nlohmann::json const& config) const;

    private:
        vector<box<Importer>> _importers;
    };
} // namespace up
