cmake_minimum_required(VERSION 3.20)
project(recon VERSION 0.1 LANGUAGES CXX)

add_executable(potato_recon)
add_executable(potato::recon ALIAS potato_recon)

add_subdirectory(source)

set_target_properties(potato_recon PROPERTIES
    OUTPUT_NAME recon
)

up_set_common_properties(potato_recon)

target_link_libraries(potato_recon PRIVATE
    potato::librecon
    potato::libruntime
    potato::libtools
    potato::libreflex
    potato::libimport
    sqlite3
    uv
)

include(up_copy_library_import)
up_copy_library_import(assimp potato_recon)
