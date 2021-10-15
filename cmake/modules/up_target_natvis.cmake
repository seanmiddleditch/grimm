include(up_utility)

function(up_target_natvis TARGET_NAME NATVIS)
    # determine if target is an INTERFACE library; if so, we add the natvis file
    # link command as an INTERFACE dependency, otherwise we link it directly
    #
    get_target_property(target_type "${TARGET_NAME}" TYPE)
    if(target_type STREQUAL INTERFACE_LIBRARY)
        set(visibility INTERFACE)
    else()
        set(visibility PRIVATE)
    endif()

    up_path_combine(${CMAKE_CURRENT_SOURCE_DIR} ${NATVIS} NATVIS)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND ${CMAKE_GENERATOR} STREQUAL Ninja)
        # When building with Ninja, we have to manually pass this to the linker
        # for MSVC since CMake doesn't do it automatically
        #
        target_link_options("${TARGET_NAME}" "${visibility}" "/NATVIS:${NATVIS}")
    endif()

    # For MSVC with MSBuild generator, we can just add the .natvis as a source;
    # for other platforms and toolsets, this doesn't hurt anything so we don't
    # need additional checks
    #
    target_sources("${TARGET_NAME}" "${visibility}" "$<BUILD_INTERFACE:${NATVIS}>")
endfunction()
