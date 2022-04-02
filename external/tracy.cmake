FetchContent_Populate(tracy)
add_library(tracy SHARED EXCLUDE_FROM_ALL)
target_sources(tracy PRIVATE
    "${tracy_SOURCE_DIR}/TracyClient.cpp"
    "${tracy_SOURCE_DIR}/Tracy.hpp"
)
target_include_directories(tracy SYSTEM PUBLIC "${tracy_SOURCE_DIR}")
target_compile_definitions(tracy
    PUBLIC
        TRACY_ENABLE=1
        TRACY_ON_DEMAND=1
        TRACY_NO_BROADCAST=1
    PRIVATE
        TRACY_EXPORTS=1
    INTERFACE
        TRACY_IMPORTS=1
)
target_link_libraries(tracy PRIVATE ${CMAKE_DL_LIBS})
