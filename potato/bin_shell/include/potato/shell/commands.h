// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/command.h"
#include "potato/game/space.h"

namespace up::shell {
    struct PlaySceneCommand : Command {
        static constexpr CommandMeta meta{.id = CommandId{"potato.command.play_scene"}};

        explicit PlaySceneCommand(box<Space> space) : space(std::move(space)) { }

        box<Space> space;
    };
} // namespace up::shell
