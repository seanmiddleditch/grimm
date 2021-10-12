// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class HlslImporter : public Importer {
    public:
        HlslImporter();
        ~HlslImporter() override;

        bool import(ImporterContext& ctx) override;

        string_view name() const noexcept override { return "hlsl"; }
        uint64 revision() const noexcept override { return 9; }

    private:
        bool _compile(
            ImporterContext& ctx,
            zstring_view absoluteSourcePath,
            string_view source,
            string_view logicalName,
            zstring_view entryName,
            zstring_view targetProfileName);
    };
} // namespace up
