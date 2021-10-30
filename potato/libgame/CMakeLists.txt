cmake_minimum_required(VERSION 3.20)
project(potato_libgame VERSION 0.1 LANGUAGES CXX)

add_library(potato_libgame)
add_library(potato::libgame ALIAS potato_libgame)

include(up_compile_sap)
include(up_set_common_properties)
up_set_common_properties(potato_libgame)

add_subdirectory(include/potato/game)
add_subdirectory(source)

up_compile_sap(potato_libgame
    SCHEMAS
        "schema/ecs.sap"
        "schema/components.sap"
)

target_link_libraries(potato_libgame
    PUBLIC
        potato::libruntime
        potato::libaudio
        potato::libreflex
        potato::librender
        potato::spud
        glm
)

add_subdirectory(tests)
