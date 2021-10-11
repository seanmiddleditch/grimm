#pragma once

#if defined(UP_REFLEX_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_REFLEX_API __declspec(dllexport)
#    else
#        define UP_REFLEX_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_REFLEX_API
#endif
