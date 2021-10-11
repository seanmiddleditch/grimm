cmake_minimum_required(VERSION 3.20)
project(runtime VERSION 0.1 LANGUAGES CXX)

add_library(potato_libruntime)
add_library(potato::libruntime ALIAS potato_libruntime)

target_sources(potato_libruntime PRIVATE
    # Callstack implementations
    #
    $<$<NOT:$<PLATFORM_ID:Windows>>:source/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/callstack.windows.cpp>

    # Debug handling implementations
    #
    $<$<NOT:$<PLATFORM_ID:Windows>>:source/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.rc>

    # Threading backends
    #
    $<$<PLATFORM_ID:Darwin>:source/thread_util.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:source/thread_util.linux.cpp>
    $<$<PLATFORM_ID:Windows>:source/thread_util.windows.cpp>

    # UUID generation
    #
    $<$<PLATFORM_ID:Darwin>:source/uuid.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:source/uuid.linux.cpp>
    $<$<PLATFORM_ID:Windows>:source/uuid.windows.cpp>

    # General runtime code
    #
    "source/asset_loader.cpp"
    "source/filesystem.cpp"
    "source/io_loop.cpp"
    "source/resource_manifest.cpp"
    "include/potato/runtime/asset.h"
    "include/potato/runtime/asset_loader.h"
    "include/potato/runtime/io_loop.h"
    "include/potato/runtime/resource_manifest.h"
    source/debug.cpp
    source/json.cpp
    source/logger.cpp
    source/path.cpp
    source/stream.cpp
    source/task_worker.cpp
    source/thread_util.cpp
    source/uuid.cpp
 )

include(up_set_common_properties)
up_set_common_properties(potato_libruntime)

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

add_executable(potato_libruntime_test)
target_sources(potato_libruntime_test PRIVATE
    "tests/main.cpp"
    "tests/test_callstack.cpp"
    "tests/test_concurrent_queue.cpp"
    "tests/test_filesystem.cpp"
    "tests/test_path_util.cpp"
    "tests/test_lock_free_queue.cpp"
    "tests/test_rwlock.cpp"
    "tests/test_task_worker.cpp"
    "tests/test_thread_util.cpp"
    "tests/test_uuid.cpp"
    "tests/test_resource_manifest.cpp"
)

up_set_common_properties(potato_libruntime_test)

target_link_libraries(potato_libruntime_test PRIVATE
    potato::libruntime
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_libruntime_test
    # for filesystem tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"

    # because concurrency tests can deadlock (which is a test failure)
    PROPERTIES TIMEOUT 10
)
