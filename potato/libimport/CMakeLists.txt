cmake_minimum_required(VERSION 3.11)
project(potato_libimport VERSION 0.1 LANGUAGES CXX)

add_library(potato_libimport)
add_library(potato::libimport ALIAS potato_libimport)

target_sources(potato_libimport PRIVATE
    "source/copy_importer.cpp"
    "source/copy_importer.h"
    "source/folder_importer.h"
    "source/hlsl_importer.cpp"
    "source/hlsl_importer.h"
    "source/importer.cpp"
    "source/importer_context.cpp"
    "source/importer_factory.cpp"
    "source/json_importer.cpp"
    "source/json_importer.h"
    "source/material_importer.cpp"
    "source/material_importer.h"
    "source/model_importer.cpp"
    "source/model_importer.h"
    "include/potato/import/_export.h"
    "include/potato/import/importer.h"
    "include/potato/import/importer_context.h"
    "include/potato/import/importer_factory.h"
)

include(up_set_common_properties)
up_set_common_properties(potato_libimport)

include(up_compile_sap)
up_compile_sap(potato_libimport SCHEMAS
    schema/importer_configs.sap
)

target_link_libraries(potato_libimport
    PUBLIC
        potato::libruntime
        potato::spud
    PRIVATE
        potato::librender
        potato::libreflex
        flatbuffers::flatlib
        nlohmann_json::nlohmann_json
        assimp
)

find_library(D3DCOMPILER_PATH d3dcompiler)
if(D3DCOMPILER_PATH)
    target_link_libraries(potato_libimport PRIVATE ${D3DCOMPILER_PATH})
endif()
