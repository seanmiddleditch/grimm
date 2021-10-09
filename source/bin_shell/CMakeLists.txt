
cmake_minimum_required(VERSION 3.20)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_executable(potato_shell WIN32)
add_executable(potato::shell ALIAS potato_shell)
target_sources(potato_shell PRIVATE
    "private/camera.cpp"
    "private/camera_controller.cpp"
    "private/editor.cpp"
    "private/editors/asset_browser.cpp"
    "private/editors/asset_browser.h"
    "private/editors/game_editor.cpp"
    "private/editors/game_editor.h"	
    "private/editors/log_window.cpp"
    "private/editors/log_window.h"
    "private/editors/material_editor.cpp"
    "private/editors/material_editor.h"
    "private/editors/scene_editor.cpp"
    "private/editors/scene_editor.h"	
    "private/log_history.cpp"
    "private/main.cpp"
    "private/scene.cpp"
    "private/scene_doc.cpp"
    "private/settings.cpp"
    "private/settings.h"
    "private/shell_app.cpp"
    "private/ui/action.cpp"
    "private/ui/command_palette.cpp"	
    "private/ui/editor_group.cpp"	
    "private/ui/editor_group.h"	
    "private/ui/menu.cpp"	
    "public/potato/shell/_export.h"
    "public/potato/shell/log_history.h"
    "public/potato/shell/scene_doc.h"
    "public/potato/shell/ui/action.h"	
    "public/potato/shell/ui/command_palette.h"	
    "public/potato/shell/ui/menu.h"	
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
    potato::libformat
    potato::librender
    potato::libecs
    potato::libreflex
    potato::libaudio
    potato::libtools
    potato::libeditor
    potato::librecon
    glm
    SDL2
    SDL2main
    imgui
    nfd
    nlohmann_json::nlohmann_json
)

include(up_copy_library_import)
up_copy_library_import(SDL2::SDL2 potato_shell)
