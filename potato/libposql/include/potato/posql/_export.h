// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_POSQL_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_POSQL_API __declspec(dllexport)
#    else
#        define UP_POSQL_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_POSQL_API
#endif
