// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "uuid.h"
#include "assertion.h"

#include "potato/spud/platform_windows.h"

#include <rpc.h>

#if !defined(UP_PLATFORM_WINDOWS)
#    error "Unsupported platform"
#endif

auto up::UUID::generate() noexcept -> UUID {
    ::GUID temp;
    if (RPC_S_OK != UuidCreate(&temp)) {
        UP_ASSERT(false, "Failed to generate unique ID.");
    }

    UUID ret;
    static_assert(sizeof(ret) == sizeof(UUID));
    std::memcpy(ret._data.ub, &temp, sizeof(UUID));
    return ret;
}
