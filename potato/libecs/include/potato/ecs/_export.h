// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_ECS_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_ECS_API __declspec(dllexport)
#    else
#        define UP_ECS_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_ECS_API
#endif
