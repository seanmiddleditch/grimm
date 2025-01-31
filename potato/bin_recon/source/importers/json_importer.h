// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/importer.h"
#include "potato/schema/importer_configs_schema.h"

namespace up {
    using JsonImporterConfig = schema::JsonImporterConfig;

    class JsonImporter : public Importer {
    public:
        JsonImporter();
        ~JsonImporter();

        bool import(ImporterContext& ctx) override;
        string_view assetType(ImporterContext& ctx) const noexcept override;

        reflex::TypeInfo const& configType() const noexcept override;

        string_view name() const noexcept override { return "json"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
