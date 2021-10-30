// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    template <typename... Fs>
    struct overload : Fs... {
        overload(Fs&&... fs) : Fs(fs)... {}

        using Fs::operator()...;
    };
} // namespace up
