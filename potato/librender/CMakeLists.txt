cmake_minimum_required(VERSION 3.20)
project(render VERSION 0.1 LANGUAGES CXX)

find_package(Assimp REQUIRED)
find_package(SDL2 REQUIRED)

add_library(potato_librender)
add_library(potato::librender ALIAS potato_librender)

target_sources(potato_librender PRIVATE
    "source/camera.cpp"
    "source/debug_draw.cpp"
    "source/image.cpp"
    "source/material.cpp"
    "source/mesh.cpp"
    "source/renderer.cpp"
    "source/stb_impl.cpp"
    "source/texture.cpp"
    "source/null_backend/null_objects.cpp"
)

include(up_compile_shader)
up_compile_shader(potato_librender STAGE vertex HLSL "data/shaders/debug.hlsl")
up_compile_shader(potato_librender STAGE pixel HLSL "data/shaders/debug.hlsl")

include(up_set_common_properties)
up_set_common_properties(potato_librender)

include(up_copy_library_import)
up_copy_library_import(assimp potato_librender)

include(up_compile_flat_schema)
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
        dx12memalloc
)

set(UP_GPU_D3D12_SOURCES
    "source/d3d12_backend/d3d12_command_list.cpp"
    "source/d3d12_backend/d3d12_buffer.cpp"
    "source/d3d12_backend/d3d12_device.cpp"
    "source/d3d12_backend/d3d12_factory.cpp"
    "source/d3d12_backend/d3d12_resource_view.cpp"
    "source/d3d12_backend/d3d12_sampler.cpp"
    "source/d3d12_backend/d3d12_swap_chain.cpp"
    "source/d3d12_backend/d3d12_platform.cpp"
    "source/d3d12_backend/d3d12_pipeline_state.cpp"
    "source/d3d12_backend/d3d12_root_signature.cpp"
    "source/d3d12_backend/d3d12_texture.cpp"
    "source/d3d12_backend/d3d12_renderable.cpp"
    "source/d3d12_backend/d3d12_desc_heap.cpp"
    "source/d3d12_backend/d3d12_context.cpp"
)

find_library(DXGI_PATH dxgi)
find_library(D3D12_PATH d3d12)

if(D3D12_PATH AND DXGI_PATH)
    target_link_libraries(potato_librender PRIVATE ${D3D12_PATH} ${DXGI_PATH})
    target_sources(potato_librender PRIVATE ${UP_GPU_D3D12_SOURCES})
    target_compile_definitions(potato_librender PUBLIC UP_GPU_ENABLE_D3D12)
    set_target_properties(potato_librender PROPERTIES UP_GPU_ENABLE_D3D12 ON)
else()
    # HEADER_FILE_ONLY is not consistently working for me,
    # so I'm adding a fake target to get the .cpp files registered
    # but unbuilt
    #
    add_library(potato_librender_d3d12 INTERFACE)
    target_sources(potato_librender_d3d12 INTERFACE ${UP_GPU_D3D12_SOURCES})
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
