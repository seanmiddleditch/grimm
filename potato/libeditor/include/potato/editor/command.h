// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/editor/editor_common.h"
#include "potato/spud/box.h"
#include "potato/spud/flags.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class CommandManager;

    UP_DEFINE_FLAGS(CommandStatus, uint8, Default = 0, Disabled = 1 << 1, Checked = 1 << 2);
    enum class CommandResponse { Ignored, Disabled, Success };

    struct CommandMeta {
        CommandId id;
        zstring_view displayName = nullptr;
        zstring_view icon = nullptr;
        zstring_view hotkey = nullptr;
        zstring_view menu = nullptr;
    };

    struct Command { };

    template <typename CommandT>
    concept CommandType = requires(CommandT const* command) {
        { &CommandT::meta } -> same_as<CommandMeta const*>;
        { command } -> convertible_to<Command const*>;
    };

    class CommandHandlerBase {
    public:
        virtual ~CommandHandlerBase() = default;

        virtual CommandId id() const noexcept = 0;
        virtual bool condition(Command const* data) = 0;
        virtual CommandStatus status(Command const* data) = 0;
        virtual void invoke(CommandManager& commands, Command* data) = 0;
    };

    template <CommandType CommandT>
    class CommandHandler : public CommandHandlerBase {
    public:
        virtual bool condition(CommandT const& command) { return true; }
        virtual CommandStatus status(CommandT const& command) { return CommandStatus::Default; };
        virtual void invoke(CommandT& command) { }
        virtual void invoke(CommandManager&, CommandT& command) { invoke(command); }

    private:
        CommandId id() const noexcept final { return CommandT::meta.id; }
        bool condition(Command const* data) final { return condition(*static_cast<CommandT const*>(data)); }
        CommandStatus status(Command const* data) final { return status(*static_cast<CommandT const*>(data)); }
        void invoke(CommandManager& commands, Command* data) final { invoke(commands, *static_cast<CommandT*>(data)); }
    };

    class CommandScope {
    public:
        template <derived_from<CommandHandlerBase> HandlerT, typename... Args>
        void addHandler(Args&&... args);

        bool handles(CommandId id) const noexcept;
        auto condition(CommandId id, Command const* data) -> CommandResponse;
        auto status(CommandId id, Command const* data) -> CommandStatus;
        void invoke(CommandManager& commands, CommandId id, Command* data);

    private:
        vector<box<CommandHandlerBase>> _handlers;
        hash_map<CommandId, uint32> _idMap;
    };

    /// @brief Manages the list of all known commands in the system
    class CommandManager {
    public:
        template <CommandType CommandT, typename... Args>
        void addCommand(Args&&... args);

        void invalidate() noexcept { ++_version; }
        [[nodiscard]] bool refresh(uint64& lastVersion) const noexcept;

        template <callable<CommandId, CommandMeta const&> CallbackT>
        void each(CallbackT&& callback) {
            for (auto const& meta : _metadata) {
                callback(meta.id, meta);
            }
        }

        void pushScope(CommandScope& scope);
        void popScope(CommandScope& scope) noexcept;

        [[nodiscard]] auto fetchMetadata(CommandId id) -> CommandMeta const*;
        [[nodiscard]] bool condition(CommandId id);
        [[nodiscard]] auto status(CommandId id) -> CommandStatus;

        bool invoke(CommandId id, Command* data = nullptr);
        template <CommandType CommandT, typename... Args>
        bool invoke(Args&&... args) {
            CommandT command(std::forward<Args>(args)...);
            return invoke(CommandT::meta.id, &command);
        }

    private:
        [[nodiscard]] auto _lookup(CommandId id) noexcept -> Command*;

        vector<box<Command>> _commands;
        vector<CommandMeta> _metadata;
        hash_map<CommandId, uint32> _idMap;
        vector<CommandScope*> _scopeStack;
        uint64 _version = 0;
    }; // namespace up::shell

    template <derived_from<CommandHandlerBase> HandlerT, typename... Args>
    void CommandScope::addHandler(Args&&... args) {
        auto const index = static_cast<uint32>(_handlers.size());
        auto const& handler = _handlers.push_back(new_box<HandlerT>(std::forward<Args>(args)...));
        _idMap.insert(handler->id(), index);
    }

    template <CommandType CommandT, typename... Args>
    void CommandManager::addCommand(Args&&... args) {
        CommandId const id = CommandT::meta.id;
        auto const index = static_cast<uint32>(_commands.size());

        _commands.push_back(new_box<CommandT>(std::forward<Args>(args)...));
        _metadata.push_back(CommandT::meta);

        _idMap.insert(id, index);
        ++_version;
    }
} // namespace up
