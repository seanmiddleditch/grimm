FetchContent_Populate(soloud)

add_library(soloud STATIC EXCLUDE_FROM_ALL)
target_include_directories(soloud SYSTEM PUBLIC "${soloud_SOURCE_DIR}/include")
target_compile_definitions(soloud
    PRIVATE
        WITH_SDL2_STATIC=1
        WITH_NULL=1
        _CRT_SECURE_NO_WARNINGS=1
        SOLOUD_NO_ASSERTS=1 # SoLoud asserts include windows.h on Win32 _and_ have a buffer-overflow, and we can't override them
)
set_target_properties(soloud PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

target_sources(soloud PRIVATE
    ${soloud_SOURCE_DIR}/src/audiosource/wav/dr_impl.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wav.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wavstream.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/stb_vorbis.c
    ${soloud_SOURCE_DIR}/src/backend/null/soloud_null.cpp
    ${soloud_SOURCE_DIR}/src/backend/sdl/soloud_sdl2.cpp
    ${soloud_SOURCE_DIR}/src/backend/sdl2_static/soloud_sdl2_static.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_audiosource.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_bus.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_3d.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_basicops.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_faderops.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_filterops.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_getters.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_setters.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_voicegroup.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_voiceops.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_fader.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_fft.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_fft_lut.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_file.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_filter.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_misc.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_queue.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_thread.cpp
)

find_package(SDL2 REQUIRED)
target_link_libraries(soloud PRIVATE SDL2)
