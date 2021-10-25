// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "entity_manager.h"
#include "system.h"

#include "potato/spud/box.h"
#include "potato/spud/vector.h"

namespace up {
    class Space {
    public:
        UP_GAME_API Space();
        UP_GAME_API virtual ~Space();

        UP_GAME_API void start();
        UP_GAME_API void stop();

        UP_GAME_API virtual void update(float deltaTime);
        UP_GAME_API virtual void render(class RenderContext& ctx);

        // FIXME: find a better place for this...
        static UP_GAME_API void addDemoSystem(Space& space, class AudioEngine& audio);

        template <typename SystemT, typename... Args>
        System& addSystem(Args&&... args) {
            UP_ASSERT(_state == State::New);
            return *_systems.push_back(new_box<SystemT>(*this, std::forward<Args>(args)...));
        }

        EntityManager& entities() noexcept { return _entities; }

    private:
        enum class State { New, Starting, Started, Stopped };

        EntityManager _entities;
        vector<box<System>> _systems;
        State _state = State::New;
    };
} // namespace up
