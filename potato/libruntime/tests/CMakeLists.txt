add_executable(potato_libruntime_test)
target_sources(potato_libruntime_test PRIVATE
    "main.cpp"
    "test_callstack.cpp"
    "test_concurrent_queue.cpp"
    "test_filesystem.cpp"
    "test_path_util.cpp"
    "test_lock_free_queue.cpp"
    "test_rwlock.cpp"
    "test_task_worker.cpp"
    "test_thread_util.cpp"
    "test_uuid.cpp"
    "test_resource_manifest.cpp"
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
