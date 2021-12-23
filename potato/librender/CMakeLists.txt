cmake_minimum_required(VERSION 3.20)
project(render VERSION 0.1 LANGUAGES CXX)

include(up_compile_shader)
include(up_set_common_properties)
include(up_copy_library_import)
include(up_compile_flat_schema)

find_package(Assimp REQUIRED)
find_package(SDL2 REQUIRED)

add_library(potato_librender)
add_library(potato::librender ALIAS potato_librender)

target_sources(potato_librender PRIVATE
    "source/debug_draw.cpp"
    "source/image.cpp"
    "source/material.cpp"
    "source/mesh.cpp"
    "source/renderer.cpp"
    "source/stb_impl.cpp"
    "source/texture.cpp"
    "source/context.cpp"
    "include/potato/render/gpu_resource.h"
)

up_set_common_properties(potato_librender)
up_copy_library_import(assimp potato_librender)

up_compile_flat_schema(potato_librender
    NAME render
    SCHEMAS
        "schema/material.fbs"
        "schema/model.fbs"
)

include(up_compile_sap)
up_compile_sap(potato_librender
    PUBLIC
    SCHEMAS
        "schema/material.sap"
)

target_link_libraries(potato_librender
    PUBLIC
        potato::libruntime
        potato::spud
        glm
    PRIVATE
        potato::libreflex
        stb
        imgui
        SDL2
)

add_subdirectory("source/null_backend")

if(UP_BUILD_D3D11)
    find_library(DXGI_PATH dxgi REQUIRED)
    find_library(D3D11_PATH d3d11 REQUIRED)

    add_subdirectory("source/d3d11_backend")

    up_compile_shader(potato_librender
        PROFILE vs_5_0
        HLSL
            "data/shaders/debug.hlsl"
            "data/shaders/imgui.hlsl"
    )
    up_compile_shader(potato_librender
        PROFILE ps_5_0
        HLSL
            "data/shaders/debug.hlsl"
            "data/shaders/imgui.hlsl"
    )

    target_link_libraries(potato_librender PRIVATE ${D3D11_PATH} ${DXGI_PATH})
    target_sources(potato_librender PRIVATE ${UP_GPU_D3D11_SOURCES})
    target_compile_definitions(potato_librender PUBLIC UP_GPU_ENABLE_D3D11=1 UP_GPU_ENABLE_D3D=1)
    set_target_properties(potato_librender PROPERTIES UP_GPU_ENABLE_D3D11 ON)
endif()

add_executable(potato_librender_test)
target_sources(potato_librender_test PRIVATE
    "tests/main.cpp"
    "tests/gpu_null_backend.cpp"
)

up_set_common_properties(potato_librender_test)

target_link_libraries(potato_librender_test PRIVATE
    potato::librender
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_librender_test)
