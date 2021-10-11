// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "desktop.h"

#include "potato/runtime/assertion.h"

bool up::desktop::openInExternalEditor(zstring_view filename) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::openInBrowser(zstring_view url) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::openInExplorer(zstring_view folder) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::selectInExplorer(zstring_view filename) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::selectInExplorer(zstring_view folder, view<zstring_view> files) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::moveToTrash(zstring_view filename) {
    UP_UNREACHABLE("not implemented");
    return false;
}

bool up::desktop::moveToTrash(view<zstring_view> files) {
    UP_UNREACHABLE("not implemented");
    return false;
}
