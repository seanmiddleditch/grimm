// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/schema/settings_schema.h"

namespace up::shell {
    bool loadShellSettings(zstring_view filename, schema::EditorSettings& settings);
    bool saveShellSettings(zstring_view filename, schema::EditorSettings const& settings);
} // namespace up::shell
