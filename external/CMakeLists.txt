# Cache old clang-tidy, since we don't want to apply it to dependencies
#
set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")

list(LENGTH UP_DEPENDENCIES COUNT)
set(INDEX 0)

# Include all the dependencies
#
message(STATUS "Updating dependencies (this might take some time!)")
foreach(EXT ${UP_DEPENDENCIES})
    math(EXPR INDEX "${INDEX}+1")
    message(STATUS "[${INDEX}/${COUNT}] Updating ${EXT}")

    include("${EXT}.cmake")
endforeach()
message(STATUS "Finished updating dependencies")

# Restore clang-tidy to our desired settings
#
set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})

