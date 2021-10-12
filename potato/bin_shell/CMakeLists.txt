
cmake_minimum_required(VERSION 3.20)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_executable(potato_shell WIN32)
add_executable(potato::shell ALIAS potato_shell)
target_sources(potato_shell PRIVATE
    "source/camera.cpp"
    "source/camera_controller.cpp"
    "source/editor.cpp"
    "source/editors/asset_browser.cpp"
    "source/editors/asset_browser.h"
    "source/editors/game_editor.cpp"
    "source/editors/game_editor.h"	
    "source/editors/log_window.cpp"
    "source/editors/log_window.h"
    "source/editors/material_editor.cpp"
    "source/editors/material_editor.h"
    "source/editors/scene_editor.cpp"
    "source/editors/scene_editor.h"	
    "source/log_history.cpp"
    "source/main.cpp"
    "source/scene.cpp"
    "source/scene_doc.cpp"
    "source/settings.cpp"
    "source/settings.h"
    "source/shell_app.cpp"
    "source/ui/action.cpp"
    "source/ui/command_palette.cpp"	
    "source/ui/editor_group.cpp"	
    "source/ui/editor_group.h"	
    "source/ui/menu.cpp"	
    "include/potato/shell/_export.h"
    "include/potato/shell/log_history.h"
    "include/potato/shell/scene_doc.h"
    "include/potato/shell/ui/action.h"	
    "include/potato/shell/ui/command_palette.h"	
    "include/potato/shell/ui/menu.h"	
 )

set_target_properties(potato_shell PROPERTIES OUTPUT_NAME shell)

include(up_set_common_properties)
include(up_compile_sap)
up_set_common_properties(potato_shell)

up_compile_sap(potato_shell SCHEMAS
    schema/components.sap
    schema/settings.sap
)

target_link_libraries(potato_shell PRIVATE
    potato::libruntime
    potato::spud
    potato::librender
    potato::libecs
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
