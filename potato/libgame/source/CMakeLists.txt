target_sources(potato_libgame PRIVATE
    "entity_id.h"
    "shared_context.cpp"
    "space.cpp"
    "universe.cpp"
    "world.cpp"
)

add_subdirectory(systems)
