if(WIN32)
    find_package(Assimp QUIET)

    if (NOT TARGET assimp)
        FetchContent_Populate(assimp_win64_sdk)

        set(ASSIMP_ROOT_DIR ${assimp_win64_sdk_SOURCE_DIR} CACHE PATH "local AssImp path" FORCE)
        find_package(Assimp REQUIRED QUIET)
    endif()
endif()
