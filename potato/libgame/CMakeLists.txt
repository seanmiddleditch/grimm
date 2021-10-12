cmake_minimum_required(VERSION 3.20)
project(potato_libgame VERSION 0.1 LANGUAGES CXX)

add_library(potato_libgame)
add_library(potato::libgame ALIAS potato_libgame)

include(up_compile_sap)
include(up_set_common_properties)
up_set_common_properties(potato_libgame)

add_subdirectory(source)

up_compile_sap(potato_libgame
    SCHEMAS
        schema/ecs.sap
)

target_link_libraries(potato_libgame
    PUBLIC
        potato::libruntime
        potato::libreflex
        potato::spud
        glm
)

add_executable(potato_libgame_test)
target_sources(potato_libgame_test PRIVATE
    "tests/main.cpp"
    "tests/test_query.cpp"
    "tests/test_world.cpp"
)

up_compile_sap(potato_libgame_test
    SCHEMAS
        tests/test_components.sap
)

up_set_common_properties(potato_libgame_test)

target_link_libraries(potato_libgame_test PRIVATE
    potato::libgame
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_libgame_test)
