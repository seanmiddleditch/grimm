// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer.h"

namespace up {
    class IgnoreImporter : public Importer {
    public:
        IgnoreImporter() = default;

        bool import(ImporterContext& ctx) override { return true; }

        string_view name() const noexcept override { return "ignore"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up
