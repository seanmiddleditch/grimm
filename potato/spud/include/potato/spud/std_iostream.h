// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/fixed_string.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <iostream>

namespace up {
    class string;
    class string_view;
    class zstring_view;
    template <size_t S>
    class fixed_string;

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, string_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, zstring_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, string const& s) {
        return os.write(s.data(), s.size());
    }

    template <typename C, size_t S>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, fixed_string<S> const& fs) {
        os.write(fs.data(), fs.size());
        return os;
    }
} // namespace up
