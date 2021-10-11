// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/span.h"
#include "potato/spud/zstring_view.h"

namespace up::desktop {
    UP_TOOLS_API bool openInExternalEditor(zstring_view filename);
    UP_TOOLS_API bool openInBrowser(zstring_view url);
    UP_TOOLS_API bool openInExplorer(zstring_view folder);
    UP_TOOLS_API bool selectInExplorer(zstring_view filename);
    UP_TOOLS_API bool selectInExplorer(zstring_view folder, view<zstring_view> files);
    UP_TOOLS_API bool moveToTrash(zstring_view filename);
    UP_TOOLS_API bool moveToTrash(view<zstring_view> files);
} // namespace up::desktop
