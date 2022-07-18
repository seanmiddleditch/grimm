cmake_minimum_required(VERSION 3.11)
project(potato_libeditor VERSION 0.1 LANGUAGES CXX)

add_library(potato_libeditor STATIC)
add_library(potato::libeditor ALIAS potato_libeditor)

target_sources(potato_libeditor PRIVATE
    "source/asset_browser_popup.cpp"
    "source/asset_edit_service.cpp"
    "source/command.cpp"
    "source/command_palette.cpp"
    "source/editor.cpp"
    "source/hotkeys.cpp"
    "source/imgui_command.cpp"
    "source/imgui_ext.cpp"
    "source/imgui_fonts.cpp"
    "source/menu.cpp"
    "source/property_grid.cpp"
    "source/project.cpp"
    "source/workspace.cpp"
    "include/potato/editor/asset_browser_popup.h"
    "include/potato/editor/asset_edit_service.h"
    "include/potato/editor/asset_type_info.h"
    "include/potato/editor/command.h"
    "include/potato/editor/command_palette.h"
    "include/potato/editor/desktop.h"
    "include/potato/editor/editor.h"
    "include/potato/editor/editor_common.h"
    "include/potato/editor/hotkeys.h"
    "include/potato/editor/icons.h"
    "include/potato/editor/imgui_command.h"
    "include/potato/editor/imgui_ext.h"
    "include/potato/editor/imgui_fonts.h"
    "include/potato/editor/menu.h"
    "include/potato/editor/property_grid.h"
    "include/potato/editor/workspace.h"
    $<$<BOOL:${UNIX}>:source/desktop.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/desktop.windows.cpp>
)

include(up_headerify)
up_headerify(potato_libeditor "roboto_font" "data/fonts/roboto/Roboto-Regular.ttf")
up_headerify(potato_libeditor "fontawesome_font" "data/fonts/fontawesome5/fa-solid-900.ttf")

include(up_set_common_properties)
up_set_common_properties(potato_libeditor)

target_link_libraries(potato_libeditor
    PUBLIC
        potato::libruntime
        potato::librender
        potato::libreflex
        potato::spud
        SDL2::SDL2
        glm::glm
        imgui
)
