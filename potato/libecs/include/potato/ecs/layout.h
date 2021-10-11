// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/type.h"

namespace up {
    /// @brief Describes the information about how components are laid out in an Archetype
    ///
    struct LayoutRow {
        ComponentId component = ComponentId::Unknown;
        reflex::TypeInfo const* typeInfo = nullptr;
        uint16 offset = 0;
        uint16 width = 0;
    };
} // namespace up
