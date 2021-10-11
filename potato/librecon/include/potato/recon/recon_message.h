// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "recon_messages_schema.h"

#include "potato/spud/concepts.h"

namespace up {
    template <typename T>
    concept ReconMessage = requires(T const& msg) {
        { T::name }
        ->convertible_to<string_view>;
        typename T::type;
    };

    struct ReconLogMessage {
        static constexpr string_view name = "LOG"_sv;
        using type = schema::ReconLogMessage;
    };

    struct ReconManifestMessage {
        static constexpr string_view name = "MANIFEST"_sv;
        using type = schema::ReconManifestMessage;
    };

    struct ReconImportMessage {
        static constexpr string_view name = "IMPORT"_sv;
        using type = schema::ReconImportMessage;
    };

    struct ReconImportAllMessage {
        static constexpr string_view name = "IMPORT_ALL"_sv;
        using type = schema::ReconImportAllMessage;
    };
} // namespace up
