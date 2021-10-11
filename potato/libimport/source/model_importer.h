// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class ModelImporter : public Importer {
    public:
        ModelImporter();
        ~ModelImporter() override;

        bool import(ImporterContext& ctx) override;

        string_view name() const noexcept override { return "model"; }
        uint64 revision() const noexcept override { return 5; }
    };
} // namespace up
