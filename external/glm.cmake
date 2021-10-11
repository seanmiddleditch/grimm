FetchContent_Populate(glm)
set(GLM_QUIET ON CACHE BOOL "quiet glm build")
set(GLM_TEST_ENABLE OFF CACHE BOOL "disable glm tests")
add_subdirectory(${glm_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glm EXCLUDE_FROM_ALL)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

include(up_target_natvis)

up_target_natvis(glm
    # the glm.natvis included with the target version of glm is currently broken
    #
    #"${glm_SOURCE_DIR}/util/glm.natvis"

    "${CMAKE_CURRENT_SOURCE_DIR}/glm.natvis"
)
