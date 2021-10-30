# Declared dependencies
#
include(config.cmake)

# List of dependencies to include
#
set(EXTERNALS
    assimp_win64
    catch2
    imgui
    glm
    stb
    json
    flatbuffers
    sdl2_vc
    soloud
    nfd
    sapc
    sqlite
    tracy
    dx12memalloc
    libuv
    nanofmt
)

# ---------------------------- #

# Cache old clang-tidy, since we don't want to apply it to dependencies
#
set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")

# Configuration to place our deps in a local location
#
set(CMAKE_FOLDER "external")
set(UP_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(FETCHCONTENT_QUIET OFF CACHE BOOL "Disable fetch content noise")

list(LENGTH EXTERNALS COUNT)
set(INDEX 0)

# Include all the dependencies
#
message(STATUS "Updating dependencies (this might take some time!)")
foreach(EXT ${EXTERNALS})
    math(EXPR INDEX "${INDEX}+1")
    message(STATUS "[${INDEX}/${COUNT}] Updating ${EXT}")

    include("${EXT}.cmake")
endforeach()
message(STATUS "Finished updating dependencies")

# Restore clang-tidy to our desired settings
#
set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})

