// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/uuid.h"
#include "potato/spud/string.h"

namespace up {
    struct MetaFile {
        static constexpr zstring_view typeName = "potato.asset.meta"_zsv;
        static constexpr int version = 1;

        UUID uuid;
        string importerName;
        string importerSettings;

        void generate();
        auto toJson() const -> string;
        bool parseJson(string_view json);
    };
} // namespace up
