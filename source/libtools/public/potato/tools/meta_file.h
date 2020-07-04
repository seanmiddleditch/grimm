// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/uuid.h"
#include "potato/spud/string.h"

namespace up {
    struct MetaFile {
        static constexpr zstring_view typeName = "potato.asset.meta"_zsv;
        static constexpr int version = 1;

        UUID uuid;
        string importerName;
        string importerSettings;

        UP_TOOLS_API void generate();
        UP_TOOLS_API auto toJson() const -> string;
        UP_TOOLS_API bool parseJson(string_view json);
    };
} // namespace up
