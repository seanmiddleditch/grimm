FetchContent_Populate(dx12memalloc)
add_library(dx12memalloc STATIC)
target_include_directories(dx12memalloc PUBLIC "${dx12memalloc_SOURCE_DIR}/src")

message("${dx12memalloc_SOURCE_DIR}")

target_sources(dx12memalloc PRIVATE
    "${dx12memalloc_SOURCE_DIR}/src/Common.cpp"
    "${dx12memalloc_SOURCE_DIR}/src/D3D12MemAlloc.cpp"
)
set_target_properties(dx12memalloc PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(dx12memalloc PUBLIC)
