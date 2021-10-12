// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "importer_context.h"

#include "potato/reflex/type.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    struct ImporterConfig {
        ImporterConfig() = default;
        virtual ~ImporterConfig() = default;
        ImporterConfig(ImporterConfig const&) = delete;
        ImporterConfig& operator=(ImporterConfig const&) = delete;
    };

    class Importer {
    public:
        Importer() = default;
        virtual ~Importer() = default;

        Importer(Importer&&) = delete;
        Importer& operator=(Importer&&) = delete;

        virtual bool import(ImporterContext& ctx) = 0;
        virtual string_view assetType([[maybe_unused]] ImporterContext& ctx) const noexcept { return {}; }

        virtual reflex::TypeInfo const& configType() const;

        virtual string_view defaultSettings() const { return {}; }

        virtual string_view name() const noexcept = 0;
        virtual uint64 revision() const noexcept = 0;
    };
} // namespace up
