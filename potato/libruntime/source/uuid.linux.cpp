// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/uuid.h"

#include "potato/spud/platform.h"

#include <uuid/uuid.h>

#if !defined(UP_PLATFORM_LINUX)
#    error "Unsupported platform"
#endif

auto up::UUID::generate() noexcept -> UUID {
    uuid_t temp;
    uuid_generate(temp);

    UUID ret;
    static_assert(sizeof(ret) == sizeof(temp));
    std::memcpy(ret._data.ub, &temp, sizeof(ret));
    return ret;
}
