// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/path.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/platform.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/utility.h"

// returns extension, including dot, e.g. foo.txt -> .txt
// only the last extension is returned, e.g. foo.txt.gz -> .gz
up::zstring_view up::path::extension(zstring_view path) noexcept {
    auto const pos = path.find_last_of("/\\.");
    if (pos != zstring_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

up::string_view up::path::extension(string_view path) noexcept {
    auto const pos = path.find_last_of("/\\.");
    if (pos != string_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

// extension must include the dot, e.g. .txt
auto up::path::changeExtension(string_view path, string_view extension) -> string {
    UP_GUARD(extension.empty() || extension.front() == '.', string{});

    auto const sepPos = path.find_last_of("/\\.");
    auto const pos = sepPos == string_view::npos || path[sepPos] != '.' ? path.size() : sepPos;

    string_writer result;
    result.reserve(pos + extension.size());
    result.append(path.data(), pos);
    result.append(extension.data(), extension.size());
    return std::move(result).to_string();
}

// returns the filename of a path, e.g. foo/bar.txt -> bar.txt
up::string_view up::path::filename(string_view path) noexcept {
    auto const pos = path.find_last_of("/\\");
    if (pos != string_view::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

// returns the basename of a path, e.g. foo/bar.txt -> bar
up::string_view up::path::filebasename(string_view path) noexcept {
    auto const ext = extension(path);
    auto const pathWithoutExt = path.substr(0, path.size() - ext.size());
    auto const pos = pathWithoutExt.find_last_of("/\\");
    if (pos != string_view::npos) {
        return pathWithoutExt.substr(pos + 1);
    }
    return pathWithoutExt;
}

up::string_view up::path::parent(string_view path) noexcept {
    if (path.empty()) {
        return path;
    }
    auto const pos = path.find_last_of("/\\");
    if (pos == 0) {
        return path.substr(0, 1);
    }
    if (pos != string_view::npos) {
        return path.substr(0, pos);
    }
    return "/";
}

bool up::path::isParentOf(string_view folder, string_view filename) noexcept {
    // we check that the filename has the folder as a prefix, and that a directory
    // separator follows the folder (so that /abcdef/file.txt isn't considered
    // a child of /abc

    if (filename.size() <= folder.size()) {
        return false;
    }

    if (!filename.starts_with(folder)) {
        return false;
    }

    char const sep = filename[folder.size()];
    return sep == '/' || sep == '\\';
}

bool up::path::isNormalized(string_view path) noexcept {
    if (path.empty()) {
        return true;
    }

    enum class Part { Empty, Component, Slash, Dot, DotDot } mode = Part::Empty;

    for (auto const ch : path) {
        switch (mode) {
            case Part::Empty:
                if (ch == '/' || ch == '\\') {
                    mode = Part::Slash;
                }
                else if (ch == '.') {
                    mode = Part::Dot;
                }
                else {
                    mode = Part::Component;
                }
                break;
            case Part::Component:
                if (ch == '/' || ch == '\\') {
                    mode = Part::Slash;
                }
                break;
            case Part::Slash:
                if (ch == '/' || ch == '\\') {
                    return false; // disallow // in path
                }
                else if (ch == '.') {
                    mode = Part::Dot;
                }
                else {
                    mode = Part::Component;
                }
                break;
            case Part::Dot:
                if (ch == '/') {
                    return false; // disallow ./
                }
                else if (ch == '.') {
                    mode = Part::DotDot;
                }
                else {
                    mode = Part::Component;
                }
                break;
            case Part::DotDot:
                if (ch == '/') {
                    return false; // disallow ../
                }
                mode = Part::Component;
                break;
        }
    }

    // require no trailing slash or dot
    return mode == Part::Component;
}

auto up::path::normalize(string_view path, Separator sep) -> string {
    char seps[] = {to_underlying(sep), '\0'};

    if (path.empty()) {
        return string{path};
    }

    string_writer result;
    result.reserve(path.size());

    auto const popPath = [&result, &seps]() {
        string_view current = result;
        auto endPos = current.find_last_of(seps);
        if (endPos != string_view::npos) {
            result.resize(endPos);
        }
    };

    auto const hasSlash = [&result, sep]() {
        string_view current = result;
        return !current.empty() && current.back() == to_underlying(sep);
    };

    enum class Part { Empty, Component, Slash, Dot, DotDot } mode = Part::Empty;

    for (auto const ch : path) {
        switch (mode) {
            case Part::Empty:
                if (ch == '/' || ch == '\\') {
                    mode = Part::Slash;
                }
                else if (ch == '.') {
                    mode = Part::Dot;
                }
                else {
                    mode = Part::Component;
                    result.append(ch);
                }
                break;
            case Part::Component:
                if (ch == '/' || ch == '\\') {
                    mode = Part::Slash;
                }
                else {
                    result.append(ch);
                }
                break;
            case Part::Slash:
                if (ch == '/' || ch == '\\') {
                    // ignore duplicate slash
                }
                else if (ch == '.') {
                    mode = Part::Dot;
                }
                else {
                    if (!hasSlash()) {
                        result.append(to_underlying(sep));
                    }
                    result.append(ch);
                    mode = Part::Component;
                }
                break;
            case Part::Dot:
                if (ch == '/' || ch == '\\') {
                    // ignore single dots
                    mode = Part::Slash;
                }
                else if (ch == '.') {
                    mode = Part::DotDot;
                }
                else {
                    result.append('.');
                    result.append(ch);
                    mode = Part::Component;
                }
                break;
            case Part::DotDot:
                if (ch == '/' || ch == '\\') {
                    popPath();
                    mode = Part::Slash;
                }
                else {
                    result.append('.');
                    result.append('.');
                    result.append(ch);
                    mode = Part::Component;
                }
                break;
        }
    }

    switch (mode) {
        case Part::DotDot:
            popPath();
            break;
        default:
            break;
    }

    if (result.empty()) {
        result.append(to_underlying(sep));
    }

    return std::move(result).to_string();
}

auto up::path::join(view<string_view> components, Separator sep) -> string {
    std::size_t size = 0;

    for (auto sv : components) {
        // path separator
        if (!sv.empty() && size != 0) {
            ++size;
        }
        size += sv.size();
    }

    string_writer result;
    result.reserve(size);

    for (auto sv : components) {
        if (!sv.empty() && !result.empty()) {
            result.append(to_underlying(sep));
        }
        result.append(sv.data(), sv.size());
    }

    return std::move(result).to_string();
}
