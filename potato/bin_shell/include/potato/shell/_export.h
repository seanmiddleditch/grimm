// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_SHELL_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_SHELL_API __declspec(dllexport)
#    else
#        define UP_SHELL_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_SHELL_API
#endif
