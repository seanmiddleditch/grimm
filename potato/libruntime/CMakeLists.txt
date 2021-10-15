cmake_minimum_required(VERSION 3.20)
project(runtime VERSION 0.1 LANGUAGES CXX)

add_library(potato_libruntime)
add_library(potato::libruntime ALIAS potato_libruntime)

add_subdirectory(include/potato/runtime)
add_subdirectory(source)

include(up_set_common_properties)
up_set_common_properties(potato_libruntime)

include(up_target_natvis)
up_target_natvis(potato_libruntime "support/libruntime.natvis")

# setup uuid libs for different oses
if (WIN32)
    set(UP_UUID_LIBRARY "rpcrt4.lib")
elseif(APPLE)
    find_library(UP_UUID_LIBRARY CoreFoundation)
elseif(UNIX)
    find_package(libuuid REQUIRED)
    if (NOT LIBUUID_FOUND)
        message(FATAL_ERROR "You might need to run 'sudo apt-get install uuid-dev' or similar")
    endif()
    set(UP_UUID_LIBRARY ${LIBUUID_LIBRARY})
    target_include_directories(potato_libruntime PRIVATE ${LIBUUID_INCLUDE_DIR})
    target_compile_definitions(potato_libruntime PRIVATE GUID_LIBUUID)
endif()

find_package(Threads REQUIRED)

target_link_libraries(potato_libruntime
    PUBLIC
        potato::spud
        nanofmt::nanofmt
        ${UP_UUID_LIBRARY}
        Threads::Threads
        nlohmann_json::nlohmann_json
        tracy
    PRIVATE
        uv
)

target_compile_definitions(potato_libruntime
    INTERFACE UP_SPUD_ASSERT_HEADER="potato/runtime/assertion.h"
    PRIVATE   UP_SPUD_ASSERT_HEADER="assertion.h"
    PUBLIC    UP_SPUD_ASSERT=UP_ASSERT
)

add_subdirectory(tests)
