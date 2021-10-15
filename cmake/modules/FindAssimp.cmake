# https://raw.githubusercontent.com/assimp/assimp/270355f326ba80a2a3a0f64a2f3ece4932cbd987/cmake-modules/Findassimp.cmake
# modified by Sean Middleditch to use an import library, remove legacy unused features

INCLUDE(FindPackageHandleStandardArgs)

function(FindAssimp)
    if(WIN32)
        set(ASSIMP_MSVC_VERSION vc142)
        set(ASSIMP_ARCHITECTURE x64)

        set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")
        set(ASSIMP_HINTS "${ASSIMP_ROOT_DIR}" "$ENV{ProgramW6432}/Assimp" "$ENV{ASSIMPDIR}" )
        set(ASSIMP_LIBRARY_SUFFICES lib debug/lib "lib/${ASSIMP_ARCHITECTURE}")
        set(ASSIMP_BINARY_SUFFICES bin debug/bin "bin/${ASSIMP_ARCHITECTURE}")

        find_path(ASSIMP_INCLUDE_DIR
            NAMES assimp/config.h
            PATH_SUFFIXES include
            HINTS ${ASSIMP_HINTS}
        )
    
        find_library(ASSIMP_LIBRARY_RELEASE
            NAMES "assimp-${ASSIMP_MSVC_VERSION}-mt.lib"
            PATH_SUFFIXES ${ASSIMP_LIBRARY_SUFFICES}
            HINTS ${ASSIMP_HINTS}
        )

        find_file(ASSIMP_DLL_RELEASE
            NAMES "assimp-${ASSIMP_MSVC_VERSION}-mt.dll"
            PATH_SUFFIXES ${ASSIMP_BINARY_SUFFICES}
            HINTS ${ASSIMP_HINTS}
        )

        find_library(ASSIMP_LIBRARY_DEBUG
            NAMES "assimp-${ASSIMP_MSVC_VERSION}-mtd.lib"
            PATH_SUFFIXES ${ASSIMP_LIBRARY_SUFFICES}
            HINTS ${ASSIMP_HINTS}
        )

        find_file(ASSIMP_DLL_DEBUG
            NAMES "assimp-${ASSIMP_MSVC_VERSION}-mtd.dll"
            PATH_SUFFIXES ${ASSIMP_BINARY_SUFFICES}
            HINTS ${ASSIMP_HINTS}
        )

        if(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY_RELEASE)
            add_library(assimp IMPORTED SHARED GLOBAL)
            set_target_properties(assimp PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${ASSIMP_INCLUDE_DIR}
            )

            if(ASSIMP_LIBRARY_DEBUG)
                set_target_properties(assimp PROPERTIES
                    IMPORTED_LOCATION_DEBUG ${ASSIMP_DLL_DEBUG}
                    IMPORTED_LOCATION_RELEASE ${ASSIMP_DLL_RELEASE}
                    IMPORTED_IMPLIB_DEBUG ${ASSIMP_LIBRARY_DEBUG}
                    IMPORTED_IMPLIB_RELEASE ${ASSIMP_LIBRARY_RELEASE}
                    MAP_IMPORTED_CONFIG_MINSIZEREL Release
                    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                ) 
            else(ASSIMP_LIBRARY_DEBUG)
                set_target_properties(assimp PROPERTIES
                    IMPORTED_LOCATION "${ASSIMP_DLL_RELEASE}"
                    IMPORTED_IMPLIB "${ASSIMP_LIBRARY_RELEASE}"
                )
            endif(ASSIMP_LIBRARY_DEBUG)
        endif()
        
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Assimp REQUIRED_VARS ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY_RELEASE)
    
    else(WIN32)
        find_path(
            assimp_INCLUDE_DIRS
            NAMES assimp/anim.h
            HINTS
                /usr/include/
                /usr/local/include/
        )

        find_library(
            assimp_LIBRARIES
            NAMES assimp
            HINTS
                /usr/lib/
                /usr/lib64/
                /usr/local/lib/
                /usr/local/lib64/
        )

        if(assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
            add_library(assimp IMPORTED SHARED GLOBAL)
            set_target_properties(assimp PROPERTIES
                IMPORTED_LOCATION ${assimp_LIBRARIES}
                INTERFACE_INCLUDE_DIRECTORIES ${assimp_INCLUDE_DIRS}
            )
        endif (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)

        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Assimp REQUIRED_VARS assimp_INCLUDE_DIRS assimp_LIBRARIES)
    
    endif(WIN32)
endfunction(FindAssimp)

if(NOT TARGET assimp)
    FindAssimp()
endif(NOT TARGET assimp)
