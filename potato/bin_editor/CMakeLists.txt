
cmake_minimum_required(VERSION 3.20)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_executable(potato_editor WIN32)
add_executable(potato::editor ALIAS potato_editor)

set_target_properties(potato_editor PROPERTIES OUTPUT_NAME editor)

include(up_set_common_properties)
up_set_common_properties(potato_editor)

add_subdirectory(source)

include(up_compile_sap)
up_compile_sap(potato_editor
    PRIVATE
    SCHEMAS
        "schema/scene.sap"
        "schema/settings.sap"
)


target_link_libraries(potato_editor PRIVATE
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
up_copy_library_import(SDL2::SDL2 potato_editor)
