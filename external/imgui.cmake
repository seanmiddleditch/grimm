FetchContent_Populate(imgui)
add_library(imgui STATIC EXCLUDE_FROM_ALL)
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
target_sources(imgui PRIVATE
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
)
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(imgui PUBLIC
    IM_ASSERT=UP_ASSERT
    IMGUI_USER_CONFIG="potato/runtime/assertion.h"
    IMGUI_DEFINE_MATH_OPERATORS=1
)
target_link_libraries(imgui PUBLIC potato::libruntime)
