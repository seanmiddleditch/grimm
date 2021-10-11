// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_IMPORT_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_IMPORT_API __declspec(dllexport)
#    else
#        define UP_IMPORT_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_IMPORT_API
#endif
