// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/command.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

namespace up {
    CommandResponse CommandScope::condition(CommandId id, Command const* data) {
        auto const item = _idMap.find(id);
        if (item) {
            return _handlers[item->value]->condition(data) ? CommandResponse::Success : CommandResponse::Disabled;
        }
        return CommandResponse::Ignored;
    }

    auto CommandScope::status(CommandId id, Command const* data) -> CommandStatus {
        auto const item = _idMap.find(id);
        if (item && _handlers[item->value]->condition(data)) {
            return _handlers[item->value]->status(data);
        }
        return CommandStatus::Disabled;
    }

    void CommandScope::invoke(CommandManager& commands, CommandId id, Command* data) {
        auto const item = _idMap.find(id);
        if (item && _handlers[item->value]->condition(data)) {
            _handlers[item->value]->invoke(commands, data);
        }
    }

    bool CommandManager::refresh(uint64& lastVersion) const noexcept {
        auto const changed = lastVersion != _version;
        lastVersion = _version;
        return changed;
    }

    void CommandManager::pushScope(CommandScope& scope) { _scopeStack.push_back(&scope); }

    void CommandManager::popScope(CommandScope& scope) noexcept {
        UP_GUARD_VOID(!_scopeStack.empty());
        UP_GUARD_VOID(_scopeStack.back() == &scope);
        _scopeStack.pop_back();
    }

    auto CommandManager::fetchMetadata(CommandId id) -> CommandMeta const* {
        auto item = _idMap.find(id);
        if (!item) {
            return nullptr;
        }
        return &_metadata[item->value];
    }

    bool CommandManager::condition(CommandId id) {
        auto* const command = _lookup(id);
        if (command == nullptr) {
            return false;
        }

        for (size_t index = _scopeStack.size(); index != 0; --index) {
            CommandResponse const rs = _scopeStack[index - 1]->condition(id, command);
            if (rs == CommandResponse::Success) {
                return true;
            }
            if (rs == CommandResponse::Disabled) {
                return false;
            }
        }

        return false;
    }

    auto CommandManager::status(CommandId id) -> CommandStatus {
        auto* const command = _lookup(id);
        if (command == nullptr) {
            return CommandStatus::Disabled;
        }

        for (size_t index = _scopeStack.size(); index != 0; --index) {
            CommandResponse const rs = _scopeStack[index - 1]->condition(id, command);
            if (rs == CommandResponse::Success) {
                return _scopeStack[index - 1]->status(id, command);
            }
            if (rs == CommandResponse::Disabled) {
                return CommandStatus::Disabled;
            }
        }

        return CommandStatus::Disabled;
    }

    bool CommandManager::invoke(CommandId id, Command* data) {
        if (data == nullptr) {
            data = _lookup(id);
            if (data == nullptr) {
                return false;
            }
        }

        for (size_t index = _scopeStack.size(); index != 0; --index) {
            CommandResponse const rs = _scopeStack[index - 1]->condition(id, data);
            if (rs == CommandResponse::Success) {
                _scopeStack[index - 1]->invoke(*this, id, data);
                return true;
            }
            if (rs == CommandResponse::Disabled) {
                return false;
            }
        }

        return false;
    }

    auto CommandManager::_lookup(CommandId id) noexcept -> Command* {
        auto item = _idMap.find(id);
        if (!item) {
            return nullptr;
        }
        return _commands[item->value].get();
    }
} // namespace up
