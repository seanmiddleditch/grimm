cmake_minimum_required(VERSION 3.20)
project(recon VERSION 0.1 LANGUAGES CXX)

add_executable(potato_recon)
add_executable(potato::recon ALIAS potato_recon)

target_sources(potato_recon PRIVATE
    "source/asset_database.cpp"
    "source/asset_database.h"
    "source/file_hash_cache.cpp"
    "source/file_hash_cache.h"
    "source/main.cpp"
    "source/meta_file.cpp"
    "source/meta_file.h"
    "source/recon_app.cpp"
    "source/recon_app.h"
    "source/recon_config.cpp"
    "source/recon_config.h"
    "source/recon_log_sink.cpp"
    "source/recon_log_sink.h"
    "source/recon_queue.h"
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
