// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/concepts.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

namespace up::path {
    enum class Separator : char {
        Unix = '/',
        Windows = '\\',
        Normalized = Unix,
#if _WIN32
        Native = Windows
#else
        Native = Unix
#endif
    };

    // returns extension, including dot, e.g. foo.txt -> .txt
    // only the last extension is returned, e.g. foo.txt.gz -> .gz
    UP_RUNTIME_API zstring_view extension(zstring_view path) noexcept;
    UP_RUNTIME_API string_view extension(string_view path) noexcept;
    inline zstring_view extension(string const& path) noexcept { return extension(zstring_view{path}); }

    // replaces any existing extension with the requested one
    // extension must include the dot, e.g. .txt
    // if the path has no extension, adds the provided one, e.g. -> foo/bar -> foo/bar.txt
    UP_RUNTIME_API string changeExtension(string_view path, string_view extension);

    // returns the filename of a path, e.g. foo/bar.txt -> bar.txt
    UP_RUNTIME_API string_view filename(string_view path) noexcept;

    // returns the basename of a path, e.g. foo/bar.txt -> bar
    UP_RUNTIME_API string_view filebasename(string_view path) noexcept;

    // returns the parent of a path, e.g. foo/bar.txt -> foo
    // if the path is empty, returns the empty string
    // if the path is only a /, returns /
    // if the path contains no directory separator, returns /
    UP_RUNTIME_API string_view parent(string_view path) noexcept;

    // returns true if the filename is in the given folder
    UP_RUNTIME_API bool isParentOf(string_view folder, string_view filename) noexcept;

    // returns true if the path has no . or .. components
    UP_RUNTIME_API bool isNormalized(string_view path) noexcept;

    // converts a path so that isNormalized is true
    // empty path is returned as /
    // leading slash added to path, e.g. foo.txt -> /foo.txt
    // trailing slash is stripped, e.g. /foo/ -> /foo
    // duplicate slashes are condensed, e.g. /foo//bar.txt -> /foo/bar.txt
    // double-dot sections remove prior items, e.g. /foo/../bar -> /bar
    UP_RUNTIME_API string normalize(string_view path, Separator sep = Separator::Normalized);

    // joins path components together
    // adds a / between each component, e.g. foo/, bar -> foo//bar
    // result is not normalized
    // empty components are ignored, e.g. "", bar -> bar
    UP_RUNTIME_API string join(view<string_view> components, Separator sep = Separator::Normalized);

    // joins path components together
    template <typename... String>
    string join(String const&... components) requires(convertible_to<String, string_view>&&...) {
        if constexpr (sizeof...(String) != 0) {
            string_view views[] = {string_view{components}...};
            return join(views);
        }
        else {
            return {};
        }
    }

    // joins path components together
    template <typename... String>
    string join(Separator sep, String const&... components) requires(convertible_to<String, string_view>&&...) {
        if constexpr (sizeof...(String) != 0) {
            string_view views[] = {string_view{components}...};
            return join(views, sep);
        }
        else {
            return {};
        }
    }

} // namespace up::path
