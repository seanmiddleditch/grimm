// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/key.h"
#include "potato/spud/static_id.h"

namespace up {
    struct CommandId : StaticId<CommandId> { };

    struct EditorTypeId : StaticId<EditorTypeId, uint64> { };
    struct EditorId : Key<EditorId> {
        using Key::Key;
    };
} // namespace up
