include(FetchContent)

FetchContent_Declare(catch2
    GIT_REPOSITORY git://github.com/catchorg/Catch2.git
    GIT_TAG c4e3767e265808590986d5db6ca1b5532a7f3d13 #v2.13.7
)
FetchContent_Declare(imgui
    GIT_REPOSITORY git://github.com/ocornut/imgui.git
    GIT_TAG d5a4d5300055c1222585a5f6758a232bb9d22d3f
)
FetchContent_Declare(glm
    GIT_REPOSITORY git://github.com/g-truc/glm.git
    GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e #0.9.9.8
)
FetchContent_Declare(stb
    GIT_REPOSITORY git://github.com/nothings/stb.git
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
)
FetchContent_Declare(json
    GIT_REPOSITORY git://github.com/nlohmann/json.git
    GIT_TAG aa0e847e5b57a00696bdcb6a834b927231b81613 #v3.10.3
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
    GIT_REPOSITORY git://github.com/google/flatbuffers.git
    GIT_TAG 697147a2e686486424b9d15fc3e1612586a60f97 #v1.12.1
)
FetchContent_Declare(soloud
    GIT_REPOSITORY git://github.com/jarikomppa/soloud.git
    GIT_TAG c8e339fdce5c7107bdb3e64bbf707c8fd3449beb #RELEASE_20200207
)
FetchContent_Declare(nfd 
    GIT_REPOSITORY git://github.com/mlabbe/nativefiledialog.git
    GIT_TAG 67345b80ebb429ecc2aeda94c478b3bcc5f7888e #release_116
)
FetchContent_Declare(sapc
    GIT_REPOSITORY git://github.com/potatoengine/sapc.git
    GIT_TAG cfc3bb6d44e67d87d38d6c7406710bd26c1e2cb3
)
FetchContent_Declare(tracy
    GIT_REPOSITORY git://github.com/wolfpld/tracy.git
    GIT_TAG 07778badcced109b8190805fbf2d7abfaef0d3b9 #v0.7.8
)
FetchContent_Declare(libuv
    GIT_REPOSITORY git://github.com/libuv/libuv.git
    GIT_TAG 1dff88e5161cba5c59276d2070d2e304e4dcb242 #v1.41.0
)
FetchContent_Declare(nanofmt
    GIT_REPOSITORY git://github.com/seanmiddleditch/nanofmt.git
    GIT_TAG b979434328e1f0ec5e6ef977bc59a251c4bc9dfc
)
