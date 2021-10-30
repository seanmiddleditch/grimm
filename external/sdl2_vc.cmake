if(WIN32)
    find_package(SDL2 QUIET)

    if (NOT TARGET SDL2)
        FetchContent_Populate(sdl2_vc_sdk)

        set(SDL2_PATH ${sdl2_vc_sdk_SOURCE_DIR} CACHE PATH "local SDL2 path" FORCE)
        find_package(SDL2 REQUIRED QUIET)
    endif()
endif()
