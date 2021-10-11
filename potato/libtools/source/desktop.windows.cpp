// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "desktop.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/platform_windows.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

#include <Shlobj.h>
#include <shellapi.h>

bool up::desktop::openInExternalEditor(zstring_view filename) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = filename.c_str();
    info.lpVerb = "edit";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

bool up::desktop::openInBrowser(zstring_view url) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = url.c_str();
    info.lpVerb = "open";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

bool up::desktop::openInExplorer(zstring_view folder) {
    SHELLEXECUTEINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.lpFile = folder.c_str();
    info.lpVerb = "open";
    info.fMask = SEE_MASK_FLAG_NO_UI;
    info.nShow = TRUE;
    return ShellExecuteExA(&info) == TRUE;
}

static auto makeIdList(up::zstring_view name) -> up::unique_resource<__unaligned ITEMIDLIST*, ILFree> {
    return up::unique_resource<__unaligned ITEMIDLIST*, ILFree>(ILCreateFromPathA(name.c_str()));
}

bool up::desktop::selectInExplorer(zstring_view filename) {
    auto id = makeIdList(filename);
    HRESULT const hs = SHOpenFolderAndSelectItems(id.get(), 0, nullptr, 0);
    return hs == S_OK;
}

bool up::desktop::selectInExplorer(zstring_view folder, view<zstring_view> files) {
    auto folderId = makeIdList(folder);
    if (folderId.get() == nullptr) {
        return false;
    }

    vector<unique_resource<__unaligned ITEMIDLIST*, ILFree>> items;
    items.reserve(files.size());
    for (zstring_view file : files) {
        auto id = makeIdList(file);
        if (id.get() != nullptr) {
            items.push_back(std::move(id));
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast) -- there's no other sensible way to deal with this API
    HRESULT const hs =
        SHOpenFolderAndSelectItems(folderId.get(), static_cast<UINT>(items.size()), (LPCITEMIDLIST*)items.data(), 0);
    return hs == S_OK;
}

bool up::desktop::moveToTrash(zstring_view filename) {
    zstring_view files[1] = {filename};
    return moveToTrash(files);
}

bool up::desktop::moveToTrash(view<zstring_view> files) {
    if (files.empty()) {
        return false;
    }

    // API requires all file names appended into a buffer, separated by NUL, terminated by double-NUL
    size_t length = 1; /* space for the final NUL */
    for (auto const file : files) {
        length += file.size() + 1 /*NUL separator*/;
    }

    vector<char> buffer;
    buffer.reserve(length);
    for (auto const file : files) {
        buffer.insert(buffer.end(), file.begin(), file.end());
        buffer.push_back('\0'); // separator
    }
    buffer.push_back('\0'); // final NUL terminator

    SHFILEOPSTRUCTA op;
    std::memset(&op, 0, sizeof(op));
    op.wFunc = FO_DELETE;
    op.fFlags = FOF_ALLOWUNDO /* | FOF_NOCONFIRMATION*/ | FOF_RENAMEONCOLLISION;
    op.pFrom = buffer.data();

    int const rs = SHFileOperationA(&op);
    return rs != FALSE;
}
