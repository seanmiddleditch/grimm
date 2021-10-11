// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "io_result.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class string_view;
    class zstring_view;
    class string;
    class Stream;

    UP_RUNTIME_API auto readJson(Stream& stream) -> IOReturn<nlohmann::json>;
    UP_RUNTIME_API auto readJson(zstring_view filename) -> IOReturn<nlohmann::json>;

    // nlohmann overloads
    UP_RUNTIME_API void to_json(nlohmann::json& json, string_view str) noexcept;
    UP_RUNTIME_API void to_json(nlohmann::json& json, zstring_view str) noexcept;
    UP_RUNTIME_API void to_json(nlohmann::json& json, string const& str) noexcept;
    UP_RUNTIME_API void from_json(const nlohmann::json& json, string& str);
} // namespace up
