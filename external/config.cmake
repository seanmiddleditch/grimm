include(FetchContent)

FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.0
    GIT_SHALLOW ON
)
FetchContent_Declare(imgui
    GIT_REPOSITORY git://github.com/ocornut/imgui.git
    GIT_TAG d5a4d5300055c1222585a5f6758a232bb9d22d3f
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
    GIT_SHALLOW ON
)
FetchContent_Declare(stb
    GIT_REPOSITORY git://github.com/nothings/stb.git
    GIT_TAG b42009b3b9d4ca35bc703f5310eedc74f584be58
)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.1
    GIT_SHALLOW ON
)
FetchContent_Declare(sdl2_vc_sdk
    URL https://github.com/potatoengine/win-sdks/releases/download/sdl2-2.0.12-x64/SDL2-2.0.12-win-x64.7z
    URL_HASH SHA1=21EFE9F45962EF2B57DC97FE7905D1EC82670AEF
)
FetchContent_Declare(assimp_win64_sdk
    URL https://github.com/potatoengine/win-sdks/releases/download/assimp-5.0.1-x64/assimp-5.0.1-win-x64.7z
    URL_HASH SHA1=8D96964E9E6946D39E187D6740BC5D2B21408BE2
)
FetchContent_Declare(flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v1.12.0
    GIT_SHALLOW ON
)
FetchContent_Declare(soloud
    GIT_REPOSITORY https://github.com/jarikomppa/soloud.git
    GIT_TAG RELEASE_20200207
    GIT_SHALLOW ON
)
FetchContent_Declare(nfd 
    GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
    GIT_TAG release_116
    GIT_SHALLOW ON
)
FetchContent_Declare(sapc
    GIT_REPOSITORY git://github.com/potatoengine/sapc.git
    GIT_TAG cfc3bb6d44e67d87d38d6c7406710bd26c1e2cb3 #v0.16
)
FetchContent_Declare(tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.7.4
    GIT_SHALLOW ON
)
FetchContent_Declare(libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_TAG v1.41.0
    GIT_SHALLOW ON
)
FetchContent_Declare(nanofmt
    GIT_REPOSITORY git://github.com/seanmiddleditch/nanofmt.git
    GIT_TAG b979434328e1f0ec5e6ef977bc59a251c4bc9dfc
)
