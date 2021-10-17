cmake_minimum_required(VERSION 3.20)
project(potato VERSION 0.1 LANGUAGES CXX C)

set(UP_DEFAULT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
if(DEFINED ENV{UP_ROOT_DIR})
    set(UP_DEFAULT_ROOT_DIR "$ENV{UP_ROOT_DIR}")
endif()

set(UP_DEFAULT_OUTPUT_DIR "${CMAKE_BINARY_DIR}")
if(DEFINED ENV{UP_OUTPUT_DIR})
    set(UP_DEFAULT_OUTPUT_DIR "$ENV{UP_OUTPUT_DIR}")
endif()

set(UP_ROOT_DIR "${UP_DEFAULT_ROOT_DIR}" CACHE PATH "Root source directory")
set(UP_OUTPUT_DIR "${UP_DEFAULT_OUTPUT_DIR}" CACHE PATH "Output directory")
set(UP_CXXFLAGS "$ENV{UP_CXXFLAGS}" CACHE PATH "CXXFLAGS for Potato libraries and applications (not dependencies)")

set_property(GLOBAL PROPERTY UP_DEPENDENCY_LIST)

file(TO_CMAKE_PATH "${UP_ROOT_DIR}" UP_ROOT_DIR)
file(TO_CMAKE_PATH "${UP_OUTPUT_DIR}" UP_OUTPUT_DIR)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(UP_BUILD_DOCS "Build documentation" OFF)

set(UP_CLANG_TIDY "" CACHE PATH "Path to clang-tidy")

enable_testing()
include(Dart)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(UP_TEST_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

include(GNUInstallDirs)

include(depends.cmake)

add_subdirectory(external)
add_subdirectory(potato)
add_subdirectory(docs)

configure_file(
    scripts/config/tasks.vs.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/.vs/tasks.vs.json
    @ONLY
)
configure_file(
    scripts/config/launch.vs.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/.vs/launch.vs.json
    @ONLY
)
configure_file(
    scripts/config/recon.config.json.in
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/recon.config.json
    @ONLY
)
configure_file(
    scripts/config/shell.config.json.in
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shell.config.json
    @ONLY
)

add_custom_target(potato_convert_all
    COMMAND potato_recon -path "${CMAKE_CURRENT_SOURCE_DIR}/resources" -config "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/recon.config.json"
    DEPENDS potato_recon
    COMMAND echo "Conversion complete!"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMENT "Converting resources...")

if(MSVC AND NOT CLANG_FORMAT)
    string(REGEX REPLACE "Tools[/\\].*" "Tools" VC_TOOLS_PATH ${CMAKE_CXX_COMPILER})
    find_program(CLANG_FORMAT clang-format
        HINTS ${VC_TOOLS_PATH}
        PATH_SUFFIXES llvm/x64/bin
    )
    if(CLANG_FORMAT)
        message(STATUS "Found clang-format at ${CLANG_FORMAT}")
    endif()
endif()
if(MSVC AND NOT CLANG_TIDY)
    string(REGEX REPLACE "Tools[/\\].*" "Tools" VC_TOOLS_PATH ${CMAKE_CXX_COMPILER})
    find_program(CLANG_TIDY clang-tidy
        HINTS ${VC_TOOLS_PATH}
        PATH_SUFFIXES llvm/x64/bin
    )
    if(CLANG_TIDY)
        message(STATUS "Found clang-tidy at ${CLANG_TIDY}")
    endif()
endif()

if(CLANG_FORMAT)
    add_custom_target(potato_format_all
        COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/scripts/tools/run-clang-format.py" -ri "--clang-format-executable=${CLANG_FORMAT}" "${CMAKE_CURRENT_SOURCE_DIR}/potato"
        COMMAND echo "Format complete!"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Formatting code...")
endif()

if(CLANG_TIDY)
    add_custom_target(potato_tidy_fix
        COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/scripts/tools/run-clang-tidy.py" -clang-tidy-binary "${CLANG_TIDY}" -p "${CMAKE_CURRENT_BINARY_DIR}" -quiet -fix "${CMAKE_CURRENT_SOURCE_DIR}/potato" -exclude "[.]rc$$"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Tidying code...")
endif()
