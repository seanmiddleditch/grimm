
cmake_minimum_required(VERSION 3.20)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_executable(potato_shell WIN32)
add_executable(potato::shell ALIAS potato_shell)

set_target_properties(potato_shell PROPERTIES OUTPUT_NAME shell)

include(up_set_common_properties)
up_set_common_properties(potato_shell)

add_subdirectory(include/potato/shell)
add_subdirectory(source)

include(up_compile_sap)
up_compile_sap(potato_shell SCHEMAS
    "schema/components.sap"
    "schema/resource.sap"
    "schema/scene.sap"
    "schema/settings.sap"
)


target_link_libraries(potato_shell PRIVATE
    potato::libruntime
    potato::spud
    potato::librender
    potato::libgame
    potato::libreflex
    potato::libaudio
    potato::libeditor
    potato::librecon
    nanofmt::nanofmt
    glm
    SDL2
    SDL2main
    imgui
    nfd
    nlohmann_json::nlohmann_json
)

include(up_copy_library_import)
up_copy_library_import(SDL2::SDL2 potato_shell)
