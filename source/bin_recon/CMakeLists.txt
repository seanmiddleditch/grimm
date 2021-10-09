cmake_minimum_required(VERSION 3.20)
project(recon VERSION 0.1 LANGUAGES CXX)

add_executable(potato_recon)
add_executable(potato::recon ALIAS potato_recon)

target_sources(potato_recon PRIVATE
    "private/asset_database.cpp"
    "private/asset_database.h"
    "private/file_hash_cache.cpp"
    "private/file_hash_cache.h"
    "private/main.cpp"
    "private/meta_file.cpp"
    "private/meta_file.h"
    "private/recon_app.cpp"
    "private/recon_app.h"
    "private/recon_config.cpp"
    "private/recon_config.h"
    "private/recon_log_sink.cpp"
    "private/recon_log_sink.h"
    "private/recon_queue.h"
)

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
    potato::libposql
    uv
)

include(up_copy_library_import)
up_copy_library_import(assimp potato_recon)
