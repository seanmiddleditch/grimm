if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND ${CMAKE_GENERATOR} STREQUAL Ninja)
    # When building with Ninja, we have to manually pass this to the linker
    # for MSVC since CMake doesn't do it automatically
    target_link_options(glm INTERFACE
        #"/NATVIS:${glm_SOURCE_DIR}/util/glm.natvis"
        "/NATVIS:${CMAKE_CURRENT_SOURCE_DIR}/glm.natvis"
    )
else()
    # For MSVC with MSBuild generator, we can just add the .natvis as a source;
    # for other platforms and toolsets, this doesn't hurt anything so we don't
    # need additional checks
    target_sources(foundation INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/glm.natvis
    )
endif()
