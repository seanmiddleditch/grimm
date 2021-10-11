// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    enum class ActionId : uint64 { None = ~uint64{0} };

    using ActionPredicate = delegate<bool()>;
    using ActionAction = delegate<void()>;

    class Actions;

    struct ActionDesc {
        string name;
        string command;
        string menu;
        char8_t const* icon = nullptr;
        string group;
        int priority = 1000;
        string hotKey;
        ActionPredicate enabled;
        ActionPredicate checked;
        ActionAction action;
    };

    /// @brief Provides a list of actions to the registry
    class ActionGroup {
    public:
        ActionGroup() = default;
        ~ActionGroup();

        ActionGroup(ActionGroup const&) = delete;
        ActionGroup& operator=(ActionGroup const&) = delete;

        void addAction(ActionDesc desc);

    private:
        vector<ActionDesc> _actions;
        Actions* _owner = nullptr;

        friend class Actions;
    };

    /// @brief Manages the list of all known actions in the system
    class Actions {
    public:
        using BuildCallback = delegate<void(ActionId id, ActionDesc const& action)>;

        auto addGroup(ActionGroup* group) -> bool;
        auto removeGroup(ActionGroup const* group) -> bool;

        void invalidate() noexcept { ++_version; }
        bool refresh(uint64& lastVersion) noexcept;
        void build(BuildCallback callback);

        [[nodiscard]] auto actionAt(ActionId id) const noexcept -> ActionDesc const& {
            return _groups[groupOf(id)]->_actions[indexOf(id)];
        }

        [[nodiscard]] auto isEnabled(ActionId id) -> bool;
        [[nodiscard]] auto isChecked(ActionId id) -> bool;

        bool tryInvoke(ActionId id);
        void invoke(ActionId id);

    private:
        [[nodiscard]] static constexpr auto groupOf(ActionId id) noexcept -> size_t {
            return (static_cast<uint64>(id) >> 32) & 0xFFFFFFFF;
        }
        [[nodiscard]] static constexpr auto indexOf(ActionId id) noexcept -> size_t {
            return static_cast<uint64>(id) & 0xFFFFFFFF;
        }
        [[nodiscard]] static constexpr auto idOf(size_t group, size_t index) noexcept -> ActionId {
            return static_cast<ActionId>((group << 32) | (index & 0xFFFFFFFF));
        }

        vector<ActionGroup*> _groups;
        uint64 _version = 0;

    }; // namespace up::shell
} // namespace up
