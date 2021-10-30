// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"
#include "settings_schema.h"

namespace up::shell {
    UP_SHELL_API bool loadShellSettings(zstring_view filename, schema::EditorSettings& settings);
    UP_SHELL_API bool saveShellSettings(zstring_view filename, schema::EditorSettings const& settings);
} // namespace up::shell
