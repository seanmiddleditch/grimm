function(up_headerify TARGET NAME SOURCE_FILE)
    set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
    set(OUT_HEADER_DIR "${OUT_ROOT_DIR}/inc")
    set(OUT_HEADER "${OUT_HEADER_DIR}/potato/data/${NAME}.h")

    file(MAKE_DIRECTORY "${OUT_HEADER_DIR}")
    add_custom_command(
        OUTPUT "${OUT_HEADER}"
        COMMAND potato::bin_headerify "${SOURCE_FILE}" "${OUT_HEADER}" "${NAME}"
        MAIN_DEPENDENCY "${SOURCE_FILE}"
        DEPENDS potato::bin_headerify
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND_EXPAND_LISTS
    )
    target_sources(${TARGET} PRIVATE
        "${OUT_HEADER}"
    )
    target_include_directories(${TARGET} PRIVATE "${OUT_HEADER_DIR}")
endfunction()
