cmake_minimum_required(VERSION 3.20)
project(format VERSION 0.1 LANGUAGES CXX)

add_library(potato_libformat INTERFACE)
add_library(potato::libformat ALIAS potato_libformat)

include(up_set_common_properties)
up_set_common_properties(potato_libformat)

target_link_libraries(potato_libformat
    INTERFACE
        potato::spud
)

add_executable(potato_libformat_test)
target_sources(potato_libformat_test PRIVATE
    "tests/main.cpp"
    "tests/test_format.cpp"
    "tests/test_writer.cpp")

up_set_common_properties(potato_libformat_test)

target_link_libraries(potato_libformat_test PRIVATE
    potato::libformat
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_libformat_test)
