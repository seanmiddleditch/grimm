FetchContent_Populate(sqlite_source)

add_library(sqlite3 STATIC EXCLUDE_FROM_ALL
    "${sqlite_source_SOURCE_DIR}/sqlite3.c"
    "${sqlite_source_SOURCE_DIR}/sqlite3.h"
    "${sqlite_source_SOURCE_DIR}/sqlite3ext.h"
)
target_include_directories(sqlite3 SYSTEM PUBLIC ${sqlite_source_SOURCE_DIR})
set_target_properties(sqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
target_link_libraries(sqlite3 PUBLIC ${CMAKE_DL_LIBS})
