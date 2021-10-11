// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class MaterialImporter : public Importer {
    public:
        MaterialImporter();
        ~MaterialImporter() override;

        bool import(ImporterContext& ctx) override;
        string_view assetType(ImporterContext&) const noexcept override { return "potato.asset.material"_sv; }

        string_view name() const noexcept override { return "material"_sv; }
        uint64 revision() const noexcept override { return 4; }
    };
} // namespace up
