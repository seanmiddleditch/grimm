FetchContent_Populate(json)
set(JSON_MultipleHeaders ON CACHE INTERNAL "use multiple json header build")
set(JSON_BuildTests OFF CACHE INTERNAL "disable json tests")
add_subdirectory(${json_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/json EXCLUDE_FROM_ALL)

set(nlohmann_json_DIR ${CMAKE_CURRENT_BINARY_DIR}/json CACHE FILEPATH "JSON path" FORCE)
