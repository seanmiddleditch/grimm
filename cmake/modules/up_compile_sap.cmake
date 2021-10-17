include(up_utility)
include(up_target_shortname)

cmake_policy(PUSH)

# CMP0116 must be explicitly set to disable warnings (as of CMake 3.21)
# https://cmake.org/cmake/help/v3.20/policy/CMP0116.html
#
cmake_policy(SET CMP0116 NEW)

function(up_compile_sap TARGET)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "PUBLIC;PRIVATE" "" "SCHEMAS")

    if(NOT ARG_PUBLIC AND NOT ARG_PRIVATE)
        message(FATAL_ERROR "One of PUBLIC or PRIVATE must be set for up_compile_sap")
    endif()

    set(GEN_TGT "generate_sap_schemas_${TARGET}")
    set(JSON_FILES)

    up_get_target_shortname(${TARGET} SHORT_NAME)

    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    if(${TARGET_TYPE} STREQUAL INTERFACE_LIBRARY)
        target_include_directories(${TARGET} INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
    else()
        target_include_directories(${TARGET}
            PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/gen/inc"
            PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/gen/src"
        )
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/sap")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/src/potato/schema")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/inc/potato/schema")

    foreach(FILE ${ARG_SCHEMAS})
        get_filename_component(FILE_NAME ${FILE} NAME_WE)

        set(SCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        set(JSON_FILE "gen/sap/${FILE_NAME}.json")
        set(DEP_FILE "gen/sap/${FILE_NAME}.json.d")

        list(APPEND JSON_FILES "${CMAKE_CURRENT_BINARY_DIR}/${JSON_FILE}")

        set(GENERATED_SOURCE_FILE "gen/src/${FILE_NAME}_gen.cpp")

        if(ARG_PUBLIC)
            set(GENERATED_HEADER_FILE "gen/inc/potato/schema/${FILE_NAME}_schema.h")
        else()
            set(GENERATED_HEADER_FILE "gen/src/potato/schema/${FILE_NAME}_schema.h")
        endif()

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
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        add_custom_command(
            OUTPUT "${GENERATED_HEADER_FILE}"
            COMMAND potato_bin_codegen
                    -i "${JSON_FILE}"
                    -o "${GENERATED_HEADER_FILE}"
                    -m schema_header
                    -D SHORT_NAME "${SHORT_NAME}"
                    -D EXPORT_HEADER "potato/${SHORT_NAME}/_export.h"
                    -D EXPORT_MACRO "UP_$<UPPER_CASE:${SHORT_NAME}>_API"
            MAIN_DEPENDENCY "${JSON_FILE}"
            DEPENDS potato_bin_codegen
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        add_custom_command(
            OUTPUT "${GENERATED_SOURCE_FILE}"
            COMMAND potato_bin_codegen
                    -i "${JSON_FILE}"
                    -o "${GENERATED_SOURCE_FILE}"
                    -m schema_source
                    -D MODULE_HEADER "potato/schema/${FILE_NAME}_schema.h"
            MAIN_DEPENDENCY "${JSON_FILE}"
            DEPENDS potato_bin_codegen
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        
        target_sources(${TARGET} PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/${JSON_FILE}"
            "${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_SOURCE_FILE}"
            "${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_HEADER_FILE}"
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
