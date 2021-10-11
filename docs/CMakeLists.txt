if(UP_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(UP_DOXYGEN_DOCS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(UP_DOXYGEN_SOURCE_DIR ${UP_ROOT_DIR}/source)
    set(UP_DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
    set(UP_DOXYGEN_INDEX_FILE ${UP_DOXYGEN_OUTPUT_DIR}/html/index.html)
    set(UP_DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(UP_DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    file(GLOB_RECURSE UP_PUBLIC_HEADERS ${UP_DOXYGEN_SOURCE_DIR}/*/*/include/potato/*/*.h)
 
    configure_file(${UP_DOXYFILE_IN} ${UP_DOXYFILE} @ONLY)
    add_custom_command(
        OUTPUT ${UP_DOXYGEN_INDEX_FILE}
        COMMAND ${DOXYGEN_EXECUTABLE} ${UP_DOXYFILE}
        WORKING_DIRECTORY ${UP_DOXYGEN_DOCS_DIR}
        DEPENDS ${UP_PUBLIC_HEADERS}
        MAIN_DEPENDENCY ${UP_DOXYFILE} ${UP_DOXYFILE_IN}
        COMMENT "Generating Doxygen documentation"
    )
    add_custom_target(doxygen ALL DEPENDS ${UP_DOXYGEN_INDEX_FILE})
endif(UP_BUILD_DOCS)
