// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/native.h"
#include "potato/filesystem/path.h"
#include "potato/foundation/platform.h"
#include "potato/foundation/unique_resource.h"
#include "potato/foundation/string_writer.h"
#include "potato/foundation/span.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ftw.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>

#if !UP_PLATFORM_POSIX
#    error "Invalid platform"
#endif

static auto errnoToResult(int error) noexcept -> up::IOResult {
    switch (error) {
    case 0: return up::IOResult::Success;
    default: return up::IOResult::Unknown;
    }
}

bool up::NativeFileSystem::fileExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) != 0;
}

bool up::NativeFileSystem::directoryExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode) != 0;
}

auto up::NativeFileSystem::fileStat(zstring_view path, FileStat& outInfo) const -> IOResult {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return errnoToResult(errno);
    }

    outInfo.size = st.st_size;
    outInfo.mtime = st.st_mtime;
    outInfo.type = S_ISREG(st.st_mode) ? FileType::Regular : S_ISDIR(st.st_mode) ? FileType::Directory : S_ISLNK(st.st_mode) ? FileType::SymbolicLink : FileType::Other;
    return IOResult::Success;
}

static auto enumerateWorker(up::zstring_view path, up::EnumerateCallback cb, up::string_writer& writer) -> up::EnumerateResult {
    up::unique_resource<DIR*, &closedir> dir(opendir(path.c_str()));

    auto writerPos = writer.size();

    for (struct dirent* entry = readdir(dir.get()); entry != nullptr; entry = readdir(dir.get())) {
        // skip . and ..
        if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // FIXME: don't recopy bytes every iteration, and don't allocate a whole
        // path per recursive entry
        writer.resize(writerPos);
        if (!writer.empty()) {
            writer.write('/');
        }
        writer.write(entry->d_name);

        up::FileInfo info;
        info.path = writer.c_str();
        info.size = 0;
        info.type = entry->d_type == DT_REG ? up::FileType::Regular : entry->d_type == DT_DIR ? up::FileType::Directory : entry->d_type == DT_LNK ? up::FileType::SymbolicLink : up::FileType::Other;

        struct stat st;
        if (stat(writer.c_str(), &st) == 0) {
            info.size = st.st_size;
        }

        auto result = cb(info);
        if (result == up::EnumerateResult::Break) {
            return result;
        }

        if (entry->d_type == DT_DIR && result == up::EnumerateResult::Recurse) {
            auto recurse = enumerateWorker(writer.c_str(), cb, writer);
            if (recurse == up::EnumerateResult::Break) {
                return recurse;
            }
        }
    }

    return up::EnumerateResult::Continue;
}

auto up::NativeFileSystem::enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts) const -> EnumerateResult {
    string_writer writer;

    if ((opts & EnumerateOptions::FullPath) == EnumerateOptions::FullPath) {
        writer.write(path);
    }

    return enumerateWorker(path, cb, writer);
}

auto up::NativeFileSystem::createDirectories(zstring_view path) -> IOResult {
    string dir;

    while (!path.empty() && strcmp(path.c_str(), "/") != 0 && !directoryExists(path)) {
        if (mkdir(path.c_str(), S_IRWXU) != 0) {
            return errnoToResult(errno);
        }

        dir = up::path::parent(path);
        path = dir.c_str();
    }

    return IOResult::Success;
}

auto up::NativeFileSystem::copyFile(zstring_view from, zstring_view to) -> IOResult {
    up::unique_resource<int, &close> inFile(open(from.c_str(), O_RDONLY));
    up::unique_resource<int, &close> outFile(open(to.c_str(), O_WRONLY | O_CREAT, S_IRWXU));

    up::byte buffer[32768];

    for (;;) {
        ssize_t rs = read(inFile.get(), buffer, sizeof(buffer));
        if (rs < 0) {
            return errnoToResult(errno);
        }

        if (rs == 0) {
            return IOResult::Success;
        }

        ssize_t rs2 = write(outFile.get(), buffer, rs);
        if (rs2 != rs) {
            return errnoToResult(errno);
        }
    }
}

auto up::NativeFileSystem::remove(zstring_view path) -> IOResult {
    if (::remove(path.c_str()) != 0) {
        return errnoToResult(errno);
    }
    return IOResult::Success;
}

auto up::NativeFileSystem::removeRecursive(zstring_view path) -> IOResult {
    auto cb = [](char const* path, struct stat const* st, int flags, struct FTW* ftw) -> int {
        return ::remove(path);
    };
    int rs = nftw(path.c_str(), +cb, 64, FTW_DEPTH | FTW_PHYS);
    if (rs != 0) {
        return errnoToResult(errno);
    }
    return IOResult::Success;
}

auto up::NativeFileSystem::currentWorkingDirectory() const noexcept -> string {
    // FIXME: https://eklitzke.org/path-max-is-tricky
    char buffer[PATH_MAX] = {
        0,
    };
    getcwd(buffer, sizeof(buffer));
    return string(buffer);
}

void up::NativeFileSystem::currentWorkingDirectory(zstring_view path) {
    chdir(path.c_str());
}
