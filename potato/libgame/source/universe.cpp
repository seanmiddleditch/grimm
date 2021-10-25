// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/universe.h"

#include "potato/game/shared_context.h"
#include "potato/game/world.h"
#include "potato/runtime/assertion.h"

up::Universe::Universe() : _context(new_shared<EcsSharedContext>()) { }
up::Universe::~Universe() = default;

void up::Universe::_registerComponent(ComponentId componentId, ComponentTypeBase const& componentType) {
    UP_ASSERT(_context->findComponentTypeById(componentId) == nullptr);
    _context->components.push_back({componentId, &componentType});
}
