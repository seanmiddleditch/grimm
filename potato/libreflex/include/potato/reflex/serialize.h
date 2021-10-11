// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "schema.h"

#include <nlohmann/json_fwd.hpp>

namespace up::reflex {
    template <has_schema T>
    bool encodeToJson(nlohmann::json& json, T const& value) {
        return encodeToJsonRaw(json, getSchema<T>(), &reinterpret_cast<char const&>(value));
    }

    template <has_schema T>
    bool decodeFromJson(nlohmann::json const& json, T& value) {
        return decodeFromJsonRaw(json, getSchema<T>(), &reinterpret_cast<char&>(value));
    }

    UP_REFLEX_API bool encodeToJsonRaw(nlohmann::json& json, Schema const& schema, void const* memory);
    UP_REFLEX_API bool decodeFromJsonRaw(nlohmann::json const& json, Schema const& schema, void* memory);
} // namespace up::reflex
