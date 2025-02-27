include(up_require)
include(FetchContent)

up_require(libbullet3_cmake
    GIT_REPOSITORY https://github.com/potatoengine/libbullet3-cmake.git
    GIT_COMMIT f27bf2d0d0d50051df8380b45d0b000b0ab9056a
)
up_require(sqlite_source
    URL https://www.sqlite.org/2021/sqlite-amalgamation-3360000.zip
    URL_HASH SHA3_256=d25609210ec93b3c8c7da66a03cf82e2c9868cfbd2d7d866982861855e96f972
    CMAKE_FILE sqlite.cmake
)
up_require(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_COMMIT c4e3767e265808590986d5db6ca1b5532a7f3d13
    GIT_TAG v2.13.7
    CMAKE_FILE catch2.cmake
)
up_require(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_COMMIT 9b0c26b0b2adae3ccf66dc9552fae4945d735a0c
    CMAKE_FILE imgui.cmake
)
up_require(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_COMMIT bf71a834948186f4097caa076cd2663c69a10e1e
    GIT_TAG 0.9.9.8
    CMAKE_FILE glm.cmake
)
up_require(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_COMMIT af1a5bc352164740c1cc1354942b1c6b72eacb8a
    CMAKE_FILE stb.cmake
)
up_require(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_COMMIT aa0e847e5b57a00696bdcb6a834b927231b81613
    GIT_TAG v3.10.3
    CMAKE_FILE json.cmake
)
up_require(sdl2_vc_sdk
    URL http://libsdl.org/release/SDL2-devel-2.0.20-VC.zip
    URL_HASH SHA1=4824400cc7ee56cc05061734fa04be081241b67c
    CMAKE_FILE sdl2_vc.cmake
)
up_require(assimp_win64_sdk
    URL https://github.com/potatoengine/win-sdks/releases/download/assimp-5.0.1-x64/assimp-5.0.1-win-x64.7z
    URL_HASH SHA1=8D96964E9E6946D39E187D6740BC5D2B21408BE2
    CMAKE_FILE assimp_win64.cmake
)
up_require(flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_COMMIT 697147a2e686486424b9d15fc3e1612586a60f97
    GIT_TAG v1.12.1
    CMAKE_FILE flatbuffers.cmake
)
up_require(soloud
    GIT_REPOSITORY https://github.com/jarikomppa/soloud.git
    GIT_COMMIT c8e339fdce5c7107bdb3e64bbf707c8fd3449beb
    GIT_TAG RELEASE_20200207
    CMAKE_FILE soloud.cmake
)
up_require(nfd 
    GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
    GIT_COMMIT 67345b80ebb429ecc2aeda94c478b3bcc5f7888e
    GIT_TAG release_116
    CMAKE_FILE nfd.cmake
)
up_require(sapc
    GIT_REPOSITORY https://github.com/potatoengine/sapc.git
    GIT_COMMIT cfc3bb6d44e67d87d38d6c7406710bd26c1e2cb3
)
up_require(tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_COMMIT 07778badcced109b8190805fbf2d7abfaef0d3b9
    GIT_TAG v0.7.8
    CMAKE_FILE tracy.cmake
)
up_require(libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_COMMIT 1dff88e5161cba5c59276d2070d2e304e4dcb242
    GIT_TAG v1.41.0
    CMAKE_FILE libuv.cmake
)
up_require(nanofmt
    GIT_REPOSITORY https://github.com/seanmiddleditch/nanofmt.git
    GIT_COMMIT b979434328e1f0ec5e6ef977bc59a251c4bc9dfc
)
if(UP_BUILD_D3D12)
    up_require(dx12memalloc
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git
        GIT_COMMIT 23e4d91f66397b49e84115e5d1b91fd64c3df24a
        CMAKE_FILE dx12memalloc.cmake
    )
endif()


