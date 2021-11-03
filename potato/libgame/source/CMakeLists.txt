target_sources(potato_libgame PRIVATE
    "entity_manager.cpp"
    "space.cpp"
)

add_subdirectory(components)
add_subdirectory(systems)
