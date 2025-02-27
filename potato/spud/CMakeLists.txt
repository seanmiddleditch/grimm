cmake_minimum_required(VERSION 3.20)
project(spud VERSION 0.1 LANGUAGES CXX)

add_library(potato_spud INTERFACE)
add_library(potato::spud ALIAS potato_spud)

#target_sources(potato_spud PRIVATE
#    "include/potato/spud/_hash_table.h"
#    "include/potato/spud/enumerate.h"
#    "include/potato/spud/erase.h"
#    "include/potato/spud/flags.h"
#    "include/potato/spud/generator.h"
#    "include/potato/spud/map.h"
#    "include/potato/spud/overload.h"
#    "include/potato/spud/result.h"
#    "include/potato/spud/key.h"
#    "include/potato/spud/static_id.h"
#)

include(up_set_common_properties)
up_set_common_properties(potato_spud NO_NAME_TEST)

include(up_target_natvis)
up_target_natvis(potato_spud "support/up_spud.natvis")

target_include_directories(potato_spud INTERFACE include/)

add_executable(potato_spud_test)
target_sources(potato_spud_test PRIVATE
    "tests/main.cpp"
    "tests/test_bit_set.cpp"
    "tests/test_delegate.cpp"
    "tests/test_delegate_ref.cpp"
    "tests/test_find.cpp"
    "tests/test_fixed_string.cpp"
    "tests/test_generator.cpp"
    "tests/test_hash.cpp"
    "tests/test_hash_map.cpp"
    "tests/test_hash_set.cpp"
    "tests/test_nameof.cpp"
    "tests/test_overload.cpp"
    "tests/test_rc.cpp"
    "tests/test_sequence.cpp"
    "tests/test_span.cpp"
    "tests/test_string.cpp"
    "tests/test_string_view.cpp"
    "tests/test_string_writer.cpp"
    "tests/test_vector.cpp"
    "tests/test_zstring_view.cpp"
)

target_link_libraries(potato_spud INTERFACE
    nanofmt::nanofmt
)

up_set_common_properties(potato_spud_test)

target_link_libraries(potato_spud_test PRIVATE
    potato::spud
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_spud_test)
