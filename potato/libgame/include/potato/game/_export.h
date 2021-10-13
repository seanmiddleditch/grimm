// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_GAME_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_GAME_API __declspec(dllexport)
#    else
#        define UP_GAME_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_GAME_API
#endif
