// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "json.h"
#include "filesystem.h"
#include "stream.h"

#include "potato/spud/string.h"
#include "potato/spud/string_view.h"

#include <nlohmann/json.hpp>

namespace {
    auto parseJson(up::string_view text) -> up::IOReturn<nlohmann::json> {
        auto json = nlohmann::json::parse(text.begin(), text.end(), nullptr, false);
        if (!json.is_object()) {
            return {up::IOResult::Malformed, {}};
        }

        return {up::IOResult::Success, std::move(json)};
    }
} // namespace

auto up::readJson(Stream& stream) -> IOReturn<nlohmann::json> {
    auto [rs, text] = readText(stream);
    if (rs != IOResult::Success) {
        return {rs, {}};
    }

    return parseJson(text);
}

auto up::readJson(zstring_view filename) -> IOReturn<nlohmann::json> {
    auto [rs, text] = fs::readText(filename);
    if (rs != IOResult::Success) {
        return {rs, {}};
    }

    return parseJson(text);
}

void up::to_json(nlohmann::json& json, string_view str) noexcept {
    json = std::string(str.data(), str.size());
}

void up::to_json(nlohmann::json& json, zstring_view str) noexcept {
    json = std::string(str.c_str());
}

void up::to_json(nlohmann::json& json, string const& str) noexcept {
    json = std::string(str.data(), str.size());
}

void up::from_json(const nlohmann::json& json, string& str) {
    str.reset();
    if (json.is_string()) {
        auto const& stdString = json.get_ref<std::string const&>();
        str = string(stdString.data(), stdString.size());
    }
}
