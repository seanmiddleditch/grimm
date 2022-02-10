cmake_minimum_required(VERSION 3.11)
project(potato_bin_headerify VERSION 0.1 LANGUAGES CXX)

add_executable(potato_bin_headerify)
add_executable(potato::bin_headerify ALIAS potato_bin_headerify)

target_sources(potato_bin_headerify PRIVATE
    "source/headerify_main.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_bin_headerify)
