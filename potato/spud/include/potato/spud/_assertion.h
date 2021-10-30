// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#if defined(UP_SPUD_ASSERT_HEADER)
#    include UP_SPUD_ASSERT_HEADER
#endif

#if !defined(UP_SPUD_ASSERT)
#    include <cassert>
#    define UP_SPUD_ASSERT(condition, message) assert((condition) && (message))
#endif
