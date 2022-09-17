include(FetchContent)

function(up_require)
    set(NAME "${ARGV0}")
    list(POP_FRONT ARGN)
    cmake_parse_arguments(ARG "" "URL;URL_HASH;GIT_REPOSITORY;GIT_TAG;GIT_COMMIT;CMAKE_FILE" "" ${ARGN})

    set(SUBBUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/${NAME}-subbuild")
    set(BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/${NAME}-build")

    get_property(DEPENDENCY_LIST GLOBAL PROPERTY UP_DEPENDENCY_LIST)
    list(APPEND DEPENDENCY_LIST "${NAME}")
    set_property(GLOBAL PROPERTY UP_DEPENDENCY_LIST "${DEPENDENCY_LIST}")

    if(ARG_CMAKE_FILE)
        set_property(GLOBAL PROPERTY "UP_DEPENDENCY_CMAKE_FILE_${NAME}" "${ARG_CMAKE_FILE}")
    endif()

    set(DESCRIPTION_PROPERTY "UP_DEPENDENCY_DESCRIPTION_${NAME}")

    if(ARG_URL AND ARG_URL_HASH)
        FetchContent_Declare(${NAME}
            URL "${ARG_URL}"
            URL_HASH "${ARG_URL_HASH}"
            #SUBBUILD_DIR "${SUBBUILD_DIR}"
            BINARY_DIR "${BINARY_DIR}"
            DOWNLOAD_EXTRACT_TIMESTAMP YES
        )
        set_property(GLOBAL PROPERTY "${DESCRIPTION_PROPERTY}" "URL ${ARG_URL}")
    elseif(ARG_GIT_REPOSITORY AND ARG_GIT_COMMIT)
        FetchContent_Declare(${NAME}
            GIT_REPOSITORY "${ARG_GIT_REPOSITORY}"
            GIT_TAG "${ARG_GIT_COMMIT}"
            #SUBBUILD_DIR "${SUBBUILD_DIR}"
            BINARY_DIR "${BINARY_DIR}"
        )
        if(ARG_GIT_TAG)
            set_property(GLOBAL PROPERTY "${DESCRIPTION_PROPERTY}" "${ARG_GIT_REPOSITORY} ${ARG_GIT_COMMIT} ${ARG_GIT_TAG}")
        else()
            set_property(GLOBAL PROPERTY "${DESCRIPTION_PROPERTY}" "${ARG_GIT_REPOSITORY} ${ARG_GIT_COMMIT}")
        endif()
    elseif(ARG_GIT_REPOSITORY AND ARG_GIT_TAG)
        FetchContent_Declare(${NAME}
            GIT_REPOSITORY "${ARG_GIT_REPOSITORY}"
            GIT_TAG "${ARG_GIT_TAG}"
            #SUBBUILD_DIR "${SUBBUILD_DIR}"
            BINARY_DIR "${BINARY_DIR}"
        )
        set_property(GLOBAL PROPERTY "${DESCRIPTION_PROPERTY}" "${ARG_GIT_REPOSITORY} ${ARG_GIT_TAG}")
    else()
        message(SEND_ERROR "Must specify either URL and URL_HASH; or GIT_REPOSITORY and either GIT_TAG or GIT_COMMIT")
    endif()
endfunction()
