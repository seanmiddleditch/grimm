find_library(libuv_LIBRARY NAMES uv libuv)
mark_as_advanced(libuv_LIBRARY)

find_path(libuv_INCLUDE_DIR NAMES uv.h)
mark_as_advanced(libuv_INCLUDE_DIR)

if (NOT libuv_LIBRARY OR NOT libuv_INCLUDE_DIR)
    FetchContent_Populate(libuv)
    add_subdirectory(${libuv_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libuv EXCLUDE_FROM_ALL)

    # Silence warnings that libuv generates (ugh)
    if(MSVC)
        target_compile_options(uv PRIVATE
            /wd4090 # const qualifiers
            /wd4244 # float to integer conversion
            /wd4245 # signed/unsigned mismatch
            /wd4267 # integer truncation
            /wd4701 # potentially unitialized value (!!!)
            /wd4702 # unreachable code
        )
    endif()
else()
    message(STATUS "Using system libuv ${libuv_LIBRARY}")

    add_library(uv INTERFACE)
    target_link_libraries(uv INTERFACE ${libuv_LIBRARY})
    target_include_directories(uv INTERFACE ${libuv_INCLUDE_DIR})
endif()
