## Upstream flatbuffers CMake file is a mess and has (impossible to fully disable)
## warnings with all modern versions of CMake.

FetchContent_Populate(flatbuffers)

add_library(flatbuffers_flatlib STATIC EXCLUDE_FROM_ALL)
add_library(flatbuffers::flatlib ALIAS flatbuffers_flatlib)
target_include_directories(flatbuffers_flatlib
    PUBLIC ${flatbuffers_SOURCE_DIR}/include
    PRIVATE ${flatbuffers_SOURCE_DIR}
)
target_compile_definitions(flatbuffers_flatlib PRIVATE
    -DFLATBUFFERS_LOCALE_INDEPENDENT=1
    -D_CRT_SECURE_NO_WARNINGS=1
)
target_compile_features(flatbuffers_flatlib PUBLIC
    cxx_std_17
)
target_sources(flatbuffers_flatlib PRIVATE
    ${flatbuffers_SOURCE_DIR}/src/code_generators.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_parser.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_text.cpp
    ${flatbuffers_SOURCE_DIR}/src/reflection.cpp
    ${flatbuffers_SOURCE_DIR}/src/util.cpp
)

add_executable(flatbuffers_flatc EXCLUDE_FROM_ALL)
add_executable(flatbuffers::flatc ALIAS flatbuffers_flatc)
target_include_directories(flatbuffers_flatc PRIVATE
    ${flatbuffers_SOURCE_DIR}
    ${flatbuffers_SOURCE_DIR}/grpc
)
target_sources(flatbuffers_flatc PRIVATE
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_cpp.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_csharp.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_dart.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_go.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_java.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_js_ts.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_php.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_python.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_lobster.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_lua.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_rust.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_fbs.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_grpc.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_json_schema.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_kotlin.cpp
    ${flatbuffers_SOURCE_DIR}/src/idl_gen_swift.cpp
    ${flatbuffers_SOURCE_DIR}/src/flatc.cpp
    ${flatbuffers_SOURCE_DIR}/src/flatc_main.cpp
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/schema_interface.h
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/cpp_generator.cc
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/go_generator.cc
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/java_generator.cc
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/python_generator.cc
    ${flatbuffers_SOURCE_DIR}/grpc/src/compiler/swift_generator.cc
)
target_link_libraries(flatbuffers_flatc PRIVATE flatbuffers::flatlib)

add_executable(flatbuffers_flathash EXCLUDE_FROM_ALL)
add_executable(flatbuffers::flathash ALIAS flatbuffers_flathash)
target_sources(flatbuffers_flathash PRIVATE
    ${flatbuffers_SOURCE_DIR}/src/flathash.cpp
)
