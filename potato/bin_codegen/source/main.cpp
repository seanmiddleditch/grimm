// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "generator.h"
#include "schema.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

using GeneratorFactory = std::function<std::unique_ptr<Generator>(GeneratorContext const&)>;

extern std::unique_ptr<Generator> newSchemaHeaderGenerator(GeneratorContext const& ctx);
extern std::unique_ptr<Generator> newSchemaSourceGenerator(GeneratorContext const& ctx);

static void usage(char const* program);
static GeneratorFactory selectGeneratorFactory(std::string_view mode);

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char const* argv[]) {
    using namespace std::literals;

    std::filesystem::path inputPath;
    std::filesystem::path outputPath;
    std::string mode;
    GeneratorConfig config;

    for (int argi = 1; argi != argc; ++argi) {
        if (argv[argi] == "-m"sv && argi < argc - 1) {
            mode = argv[++argi];
        }
        else if (argv[argi] == "-i"sv && argi < argc - 1) {
            inputPath = argv[++argi];
        }
        else if (argv[argi] == "-o"sv && argi < argc - 1) {
            outputPath = argv[++argi];
        }
        else if (argv[argi] == "-D"sv && argi < argc - 2) {
            auto const* const key = argv[++argi];
            auto const* const value = argv[++argi];
            config.insert({key, value});
        }
        else {
            std::cerr << "Unknown argument '" << argv[argi] << "'\n";
            usage(argv[0]);
            return 1;
        }
    }

    if (inputPath.empty()) {
        std::cerr << "Missing input path argument\n";
        usage(argv[0]);
        return 1;
    }

    if (outputPath.empty()) {
        std::cerr << "Missing output path argument\n";
        usage(argv[0]);
        return 1;
    }

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        std::cerr << "Failed to open `" << inputPath << "` for reading\n";
        return 2;
    }

    schema::Module mod;
    try {
        if (!schema::loadModule(input, mod)) {
            std::cerr << "Failed to parse JSON in `" << inputPath << "`\n";
            return 4;
        }
    }
    catch (nlohmann::json::exception const& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 6;
    }
    input.close();

    auto factory = selectGeneratorFactory(mode);
    if (factory == nullptr) {
        std::cerr << "Unknown mode '" << mode << "'\n";
        return 5;
    }

    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        std::cerr << "Failed to open `" << outputPath << "` for writing\n";
        return 7;
    }

    GeneratorContext ctx{output, mod, config};
    auto generator = factory(ctx);

    auto const rs = generator->generate();

    output.close();

    if (!rs) {
        std::cerr << "Generator '" << mode << "' failed\n";
        return 8;
    }

    if (generator->errors() != 0) {
        return 8;
    }

    return 0;
}

void usage(char const* program) {
    std::cerr << "Usage: " << program << " -m <mode> -i <input> -o <output>\n ";
}

GeneratorFactory selectGeneratorFactory(std::string_view mode) {
    using namespace std::literals;

    if (mode == "schema_header"sv) {
        return [](GeneratorContext const& ctx) {
            return newSchemaHeaderGenerator(ctx);
        };
    }

    if (mode == "schema_source"sv) {
        return [](GeneratorContext const& ctx) {
            return newSchemaSourceGenerator(ctx);
        };
    }

    return nullptr;
}
