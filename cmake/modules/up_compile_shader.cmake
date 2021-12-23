function(up_compile_shader TARGET)
    cmake_parse_arguments(ARG "" "PROFILE;ENTRY" "HLSL" ${ARGN})

    if(NOT ARG_PROFILE)
        message(FATAL_ERROR "up_compiler_shader requires STAGE parameter")
    endif()
    if(NOT ARG_HLSL)
        message(FATAL_ERROR "up_compiler_shader requires HLSL parameter")
    endif()
    
    if(ARG_PROFILE MATCHES "^ps_")
        set(STAGE "pixel")
    elseif(ARG_PROFILE MATCHES "^vs_")
        set(STAGE "vertex")
    else()
        message(FATAL_ERROR "Unrecognized stage for profile `${ARG_PROFILE}`")
    endif()

    if(NOT ARG_ENTRY)
        set(ARG_ENTRY "${STAGE}_main")
    endif()

    if(WIN32)
        find_program(FXC_PATH
            NAMES fxc.exe
	        HINTS $ENV{DXCDIR}
	        PATH_SUFFIXES bin
        )

        set(OUT_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
        file(MAKE_DIRECTORY "${OUT_HEADER_DIR}")

        foreach(HLSL_SHADER ${ARG_HLSL})
            get_filename_component(BASENAME "${HLSL_SHADER}" NAME_WE)
            set(OUT_HEADER "${OUT_HEADER_DIR}/potato/shader/${BASENAME}_${ARG_PROFILE}_shader.h")

            add_custom_command(
                OUTPUT "${OUT_HEADER}"
                COMMAND ${FXC_PATH} /nologo /O3 /Zi /T "${ARG_PROFILE}" /E "${ARG_ENTRY}" /Fh "${OUT_HEADER}" "${HLSL_SHADER}"
                MAIN_DEPENDENCY "${HLSL_SHADER}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
            target_sources(${TARGET} PRIVATE "${OUT_HEADER}")
            target_include_directories(${TARGET} PRIVATE "${OUT_HEADER_DIR}")
        endforeach()
    endif()
endfunction()
