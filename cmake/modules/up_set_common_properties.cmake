function(up_set_common_properties TARGET)
    cmake_parse_arguments(ARG NO_NAME_TEST "" "" ${ARGN})
    # Potato targets must start with potato_
    #
    if(NOT ${TARGET} MATCHES "^potato_")
        message(FATAL_ERROR "Target '${TARGET}' must start with potato_ prefix")
    endif()
    string(REGEX REPLACE "^(potato_)" "" SHORT_NAME ${TARGET})

    # Detect type of target
    #
    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    set(PUBLIC_INTERFACE PUBLIC)
    set(IS_INTERFACE FALSE)
    if (${TARGET_TYPE} STREQUAL INTERFACE_LIBRARY)
        set(PUBLIC_INTERFACE INTERFACE)
        set(IS_INTERFACE TRUE)
    endif()

    # Potato requires C++17
    #
    target_compile_features(${TARGET} ${PUBLIC_INTERFACE}
        cxx_std_17
    )
    if (NOT IS_INTERFACE)
        set_target_properties(${TARGET} PROPERTIES
            LINKER_LANGUAGE CXX
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
        )
    endif()

    # Enforce some naming rules for hygiene.
    #
    set(IS_TEST FALSE)
    set(IS_BINARY FALSE)
    set(IS_LIBRARY FALSE)
    if (${TARGET} MATCHES "_test$")
        set(TYPE "test")
        set(IS_TEST TRUE)

        string(REGEX REPLACE "_test$" "" TEST_TARGET ${TARGET})
        if(NOT TARGET ${TEST_TARGET})
            message(FATAL_ERROR "Test executable target '${TARGET}' expects there to be a target '${TEST_TARGET}'")
        endif()
    elseif (${TARGET_TYPE} STREQUAL "EXECUTABLE")
        set(TYPE "executable")
        set(IS_BINARY TRUE)
    elseif(${TARGET_TYPE} MATCHES "_LIBRARY")
        set(TYPE "library")
        set(IS_LIBRARY TRUE)

        if(NOT ARG_NO_NAME_TEST AND NOT ${SHORT_NAME} MATCHES "^lib")
            message(FATAL_ERROR "Library target '${TARGET}' should be named 'potato_lib${SHORT_NAME}'")
        endif()
    else()
        message(FATAL_ERROR "Target '${TARGET}' has unknown type '${TARGET_TYPE}'")
    endif()

    # Set output name
    #
    if (NOT IS_INTERFACE)
        set_target_properties(${TARGET} PROPERTIES
            ARCHIVE_OUTPUT_NAME "${SHORT_NAME}"
            LIBRARY_OUTPUT_NAME "${SHORT_NAME}"
            RUNTIME_OUTPUT_NAME "${SHORT_NAME}"
        )
    endif()

    # Trick MSVC into behaving like a standards-complient
    # compiler.
    target_compile_options(${TARGET} ${PUBLIC_INTERFACE}
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    )
    if (NOT IS_INTERFACE)
        target_compile_options(${TARGET} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/permissive->
            $<$<CXX_COMPILER_ID:MSVC>:/Zc:inline>
        )
    endif()
    
    # Enable clang-tidy
    #
    if (UP_CLANG_TIDY AND NOT IS_INTERFACE AND NOT IS_TEST)
        set_target_properties(${TARGET} PROPERTIES
            CXX_CLANG_TIDY ${UP_CLANG_TIDY}
        )
    endif()

    # Disable C4141, since __forceinline apparently implies
    # inline, but Clang/GCC's [always_inline] does _not_
    # imply inline, so it needs to be present
    target_compile_options(${TARGET} ${PUBLIC_INTERFACE}
        $<$<CXX_COMPILER_ID:MSVC>:/wd4141>
    )

    # Windows should always enable UNICODE support, though
    # we should never use TCHAR or the unadorned functions.
    # The goal is to ensure here we don't accidentally use
    # an unadorned function with our usual string types and
    # have things Just Compile(tm) incorrectly.
    #
    target_compile_definitions(${TARGET} ${PUBLIC_INTERFACE}
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    )

    # Set visibility hidden on *nix targets to mimic
    # Windows', and also for smaller symbol tables during
    # the linking stage.
    #
    if(NOT IS_INTERFACE)
        string(REGEX REPLACE "^lib|_test$" "" EXPORT_NAME ${SHORT_NAME})
        string(TOUPPER ${EXPORT_NAME} EXPORT_NAME)
        set_target_properties(${TARGET} PROPERTIES
            DEFINE_SYMBOL "UP_${EXPORT_NAME}_EXPORTS"
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )
    endif()

    # Doctest settings to make things work well.
    #
    if(IS_TEST)
        target_compile_definitions(${TARGET} PRIVATE
            DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
            DOCTEST_CONFIG_SUPER_FAST_ASSERTS
            DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
        )
    endif()

    # Folder category for IDE solution builds.
    #
    if(NOT IS_INTERFACE)
        set_target_properties(${TARGET} PROPERTIES FOLDER ${TYPE})
    endif()

    # Library public include paths and private paths
    # for both library and executable targets
    #
    target_include_directories(${TARGET} ${PUBLIC_INTERFACE}
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    )
    if(NOT IS_INTERFACE)
        target_include_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/private
        )
    endif()

    ## Set test output directory.
    ## Not actually a good idea without figuring out how to place
    ## runtime libraries next to the tests.
    ##
    #if (${TYPE} STREQUAL "test")
    #    set_target_properties(${TARGET} PROPERTIES
    #        RUNTIME_OUTPUT_DIRECTORY ${UP_TEST_OUTPUT_DIRECTORY}
    #    )
    #endif()
endfunction()
