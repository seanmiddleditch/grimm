// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class Importer;
    struct ImporterConfig;

    class ImporterFactory {
    public:
        ImporterFactory();
        ~ImporterFactory();

        ImporterFactory(ImporterFactory const&) = delete;
        ImporterFactory& operator=(ImporterFactory const&) = delete;

        Importer* findImporterByName(string_view name) const noexcept;

        void registerImporter(box<Importer> importer);

        void registerDefaultImporters();

        box<ImporterConfig> parseConfig(Importer const& importer, nlohmann::json const& config) const;

    private:
        vector<box<Importer>> _importers;
    };
} // namespace up
