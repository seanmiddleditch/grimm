// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "io_result.h"

#include "potato/spud/delegate_ref.h"
#include "potato/spud/int_types.h"
#include "potato/spud/string.h"
#include "potato/spud/utility.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Stream;
} // namespace up

namespace up::fs {
    enum class EnumerateResult {
        Next,
        Recurse,
        Stop,
    };

    constexpr EnumerateResult next = EnumerateResult::Next;
    constexpr EnumerateResult recurse = EnumerateResult::Recurse;
    constexpr EnumerateResult stop = EnumerateResult::Stop;

    enum class OpenMode { Binary, Text };

    enum class FileType { Regular, Directory, SymbolicLink, Other };

    struct Stat {
        size_t size = 0;
        uint64 mtime = 0;
        FileType type = FileType::Regular;
    };

    struct EnumerateItem {
        zstring_view path;
        size_t size = 0;
        FileType type = FileType::Regular;
    };
    using EnumerateCallback = up::delegate_ref<EnumerateResult(EnumerateItem const& item, int depth)>;

    [[nodiscard]] UP_RUNTIME_API bool fileExists(zstring_view path) noexcept;
    [[nodiscard]] UP_RUNTIME_API bool directoryExists(zstring_view path) noexcept;

    [[nodiscard]] UP_RUNTIME_API IOReturn<Stat> fileStat(zstring_view path);

    [[nodiscard]] UP_RUNTIME_API Stream openRead(zstring_view path, OpenMode mode = OpenMode::Binary);
    [[nodiscard]] UP_RUNTIME_API Stream openWrite(zstring_view path, OpenMode mode = OpenMode::Binary);

    [[nodiscard]] UP_RUNTIME_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb);

    [[nodiscard]] UP_RUNTIME_API IOResult createDirectories(zstring_view path);

    [[nodiscard]] UP_RUNTIME_API IOResult remove(zstring_view path);
    [[nodiscard]] UP_RUNTIME_API IOResult removeRecursive(zstring_view path);

    [[nodiscard]] UP_RUNTIME_API string currentWorkingDirectory() noexcept;
    [[nodiscard]] UP_RUNTIME_API bool currentWorkingDirectory(zstring_view path);

    [[nodiscard]] UP_RUNTIME_API IOResult copyFileTo(zstring_view fromPath, zstring_view toPath);
    [[nodiscard]] UP_RUNTIME_API IOResult moveFileTo(zstring_view fromPath, zstring_view toPath);
} // namespace up::fs
