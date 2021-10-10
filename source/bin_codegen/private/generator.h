#pragma once

#include <nanofmt/format.h>
#include <iosfwd>
#include <string_view>
#include <unordered_map>

namespace schema {
    struct Module;
}

using GeneratorConfig = std::unordered_map<std::string, std::string>;

struct GeneratorContext {
    std::ostream& output;
    schema::Module const& mod;
    GeneratorConfig const& config;
};

class Generator {
public:
    explicit Generator(GeneratorContext const& ctx) : _output(ctx.output), _module(ctx.mod), _config(ctx.config) {}
    virtual ~Generator() = default;

    virtual bool generate() = 0;

    int errors() const noexcept { return _errors; }

protected:
    std::ostream& _output; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    schema::Module const& _module; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    GeneratorConfig const& _config; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    void fail(std::string_view message);

    template <typename... ArgsT>
    void fail(std::string_view format_str, ArgsT const&... args) {
        char buffer[1024];
        nanofmt::format_to(buffer, {format_str.data(), format_str.size()}, args...);
        fail(buffer);
    }

    std::string_view config(std::string const& key) const noexcept {
        auto const it = _config.find(key);
        return it != _config.end() ? std::string_view{it->second} : std::string_view{};
    }

private:
    int _errors = 0;
};
