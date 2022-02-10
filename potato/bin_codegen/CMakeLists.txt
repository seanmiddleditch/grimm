cmake_minimum_required(VERSION 3.11)
project(potato_bin_codegen VERSION 0.1 LANGUAGES CXX)

add_executable(potato_bin_codegen)
add_executable(potato::bin_codegen ALIAS potato_bin_codegen)

target_sources(potato_bin_codegen PRIVATE
    "source/codegen_cxx.h"
    "source/codegen_generator.cpp"
    "source/codegen_generator.h"
    "source/codegen_main.cpp"
    "source/codegen_schema.cpp"
    "source/codegen_schema.h"
    "source/generators/schema_header_gen.cpp"
    "source/generators/schema_source_gen.cpp"
)

target_link_libraries(potato_bin_codegen PRIVATE
    nlohmann_json::nlohmann_json
    nanofmt::nanofmt
)

include(up_set_common_properties)
up_set_common_properties(potato_bin_codegen)
