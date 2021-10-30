# Cache old clang-tidy, since we don't want to apply it to dependencies
#
set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")

# Read in dependencies
get_property(DEPENDENCIES GLOBAL PROPERTY UP_DEPENDENCY_LIST)

list(LENGTH DEPENDENCIES COUNT)
set(INDEX 0)

# Include all the dependencies
#
message(STATUS "Updating ${COUNT} dependencies (this might take some time!)")
foreach(EXT ${DEPENDENCIES})
    math(EXPR INDEX "${INDEX}+1")

    get_property(EXT_DESCRIPTION GLOBAL PROPERTY "UP_DEPENDENCY_DESCRIPTION_${EXT}")
    if(NOT EXT_DESCRIPTION)
        set(EXT_DESCRIPTION "${EXT}")
    endif()

    get_property(EXT_CMAKE_FILE GLOBAL PROPERTY "UP_DEPENDENCY_CMAKE_FILE_${EXT}")
    if(NOT EXT_CMAKE_FILE)
        set(EXT_CMAKE_FILE "${EXT}.cmake")
    endif()

    message(STATUS "[${INDEX}/${COUNT}] Updating ${EXT} (${EXT_DESCRIPTION})")

    include("${EXT_CMAKE_FILE}")
endforeach()
message(STATUS "Finished updating dependencies")

# Restore clang-tidy to our desired settings
#
set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})

