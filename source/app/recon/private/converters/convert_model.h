// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class ModelConverter : public Converter {
    public:
        ModelConverter();
        ~ModelConverter();

        bool convert(Context& ctx) override;
        string generateSettings(Context& ctd) { return ""; }

        string_view name() const noexcept override { return "model"; }
        uint64 revision() const noexcept override { return 1; }
    };
} // namespace up::recon
