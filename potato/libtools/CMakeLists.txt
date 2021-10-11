cmake_minimum_required(VERSION 3.11)
project(potato_libtools VERSION 0.1 LANGUAGES CXX)

add_library(potato_libtools)
add_library(potato::libtools ALIAS potato_libtools)

target_sources(potato_libtools PRIVATE
    "source/project.cpp"
    "include/potato/tools/desktop.h"
    $<$<BOOL:${UNIX}>:source/desktop.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/desktop.windows.cpp>
 )

include(up_set_common_properties)
up_set_common_properties(potato_libtools)

target_link_libraries(potato_libtools
    PUBLIC
        potato::libruntime
        potato::libreflex
        potato::spud
)
