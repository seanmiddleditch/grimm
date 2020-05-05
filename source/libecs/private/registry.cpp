// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include "potato/ecs/registry.h"
#include "potato/ecs/component.h"
#include <potato/spud/find.h>

auto up::ComponentRegistry::defaultRegistry() noexcept -> ComponentRegistry& {
    static ComponentRegistry instance;
    return instance;
}

void up::ComponentRegistry::_registerComponent(ComponentMeta const& meta) {
    auto& newMeta = _components.push_back(meta);
    newMeta.index = _nextIndex++;
}

void up::ComponentRegistry::deregisterComponent(ComponentId id) {
    auto const* it = find(_components, id, {}, &ComponentMeta::id);
    if (it != _components.end()) {
        _components.erase(it);
    }
}

auto up::ComponentRegistry::findByName(string_view name) const noexcept -> ComponentMeta const* {
    auto const* it = find(_components, name, {}, &ComponentMeta::name);
    return it != _components.end() ? it : nullptr;
}

auto up::ComponentRegistry::findById(ComponentId id) const noexcept -> ComponentMeta const* {
    auto const* it = find(_components, id, equality{}, &ComponentMeta::id);
    return it != _components.end() ? it : nullptr;
}

auto up::ComponentRegistry::_findByType(uint64 typeHash) const noexcept -> ComponentMeta const* {
    auto const* it = find(_components, typeHash, equality{}, &ComponentMeta::typeHash);
    return it != _components.end() ? it : nullptr;
}
