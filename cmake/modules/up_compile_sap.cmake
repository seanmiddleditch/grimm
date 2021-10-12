include(up_utility)
include(up_target_shortname)

cmake_policy(PUSH)

# CMP0116 must be explicitly set to disable warnings (as of CMake 3.21)
# https://cmake.org/cmake/help/v3.20/policy/CMP0116.html
#
cmake_policy(SET CMP0116 NEW)

function(up_compile_sap TARGET)
    cmake_parse_arguments(ARG "" "" "SCHEMAS" ${ARGN})

    set(GEN_TGT "generate_sap_schemas_${TARGET}")
    set(OUT_FILES)

    up_get_target_shortname(${TARGET} SHORT_NAME)

    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    if (${TARGET_TYPE} STREQUAL INTERFACE_LIBRARY)
        target_include_directories(${TARGET} INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
    else()
        target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/sap")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/src")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")

    foreach(FILE ${ARG_SCHEMAS})
        get_filename_component(FILE_NAME ${FILE} NAME_WE)

        SET(SCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        SET(JSON_FILE "gen/sap/${FILE_NAME}.json")
        SET(DEP_FILE "gen/sap/${FILE_NAME}.json.d")

        SET(GENERATED_SOURCE_FILE "gen/src/${FILE_NAME}_gen.cpp")
        SET(GENERATED_HEADER_FILE "gen/inc/${FILE_NAME}_schema.h")

        target_sources(${TARGET} PRIVATE "${JSON_FILE}" "${GENERATED_SOURCE_FILE}" "${GENERATED_HEADER_FILE}")

        add_custom_command(
            OUTPUT "${JSON_FILE}"
            COMMAND sapc -o "${JSON_FILE}"
                    -d "${DEP_FILE}"
                    "-I$<JOIN:$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>,;-I>"
                    -- "${SCHEMA_FILE}"
            MAIN_DEPENDENCY "${SCHEMA_FILE}"
            DEPENDS sapc
            DEPFILE "${DEP_FILE}"
            COMMAND_EXPAND_LISTS
        )
        add_custom_command(
            OUTPUT "${GENERATED_HEADER_FILE}"
            COMMAND potato_bin_codegen
                    -i "${JSON_FILE}"
                    -o "${GENERATED_HEADER_FILE}"
                    -m schema_header
                    -D EXPORT_HEADER "potato/${SHORT_NAME}/_export.h"
                    -D EXPORT_MACRO "UP_$<UPPER_CASE:${SHORT_NAME}>_API"
            MAIN_DEPENDENCY "${JSON_FILE}"
            DEPENDS potato_bin_codegen
        )
        add_custom_command(
            OUTPUT "${GENERATED_SOURCE_FILE}"
            COMMAND potato_bin_codegen
                    -i "${JSON_FILE}"
                    -o "${GENERATED_SOURCE_FILE}"
                    -m schema_source
                    -D MODULE_HEADER "${FILE_NAME}_schema.h"
            MAIN_DEPENDENCY "${JSON_FILE}"
            DEPENDS potato_bin_codegen
        )
    endforeach()

    if(NOT TARGET "${GEN_TGT}")
        add_custom_target("${GEN_TGT}"
            DEPENDS ${JSON_FILES}
        )
    else()
        add_dependencies(${GEN_TGT} ${JSON_FILES})
    endif()

    add_dependencies(${TARGET} ${GEN_TGT})
endfunction()

cmake_policy(POP)
