// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "universe.h"
#include "shared_context.h"
#include "world.h"

#include "potato/runtime/assertion.h"

up::Universe::Universe() : _context(new_shared<EcsSharedContext>()) {}
up::Universe::~Universe() = default;

auto up::Universe::findComponentByName(string_view name) const noexcept -> reflex::TypeInfo const* {
    return _context->findComponentByName(name);
}

void up::Universe::_registerComponent(reflex::TypeInfo const& typeInfo) {
    UP_ASSERT(_context->_findComponentByTypeHash(typeInfo.hash) == nullptr);
    _context->components.push_back(&typeInfo);
}
