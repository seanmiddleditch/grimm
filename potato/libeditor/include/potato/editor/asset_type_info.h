// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/editor/editor.h"
#include "potato/spud/zstring_view.h"

namespace up {
    struct AssetTypeInfo {
        zstring_view name;
        zstring_view extension;
        EditorTypeId editor;
        char const* icon = nullptr;
        uint64 typeHash = 0;
    };
} // namespace up
