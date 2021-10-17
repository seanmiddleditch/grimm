add_executable(potato_libgame_test)
target_sources(potato_libgame_test PRIVATE
    "main.cpp"
    "test_query.cpp"
    "test_world.cpp"
)

up_compile_sap(potato_libgame_test
    PRIVATE
    SCHEMAS
        test_components.sap
)

up_set_common_properties(potato_libgame_test)

target_link_libraries(potato_libgame_test PRIVATE
    potato::libgame
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(potato_libgame_test)
