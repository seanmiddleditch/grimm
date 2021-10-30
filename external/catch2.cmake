FetchContent_Populate(catch2)
set(CATCH_ENABLE_WERROR OFF CACHE BOOL "disable catch2 -Werror mode")
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "disable catch2 docs")
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "disable catch2 contrib")
add_subdirectory(${catch2_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/catch2 EXCLUDE_FROM_ALL)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${catch2_SOURCE_DIR}/contrib" PARENT_SCOPE)
