// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "system.h"
#include "world.h"

#include "potato/spud/box.h"
#include "potato/spud/vector.h"

namespace up {
    class Renderer;

    class Space {
    public:
        UP_GAME_API Space(box<World> world);
        UP_GAME_API virtual ~Space();

        UP_GAME_API void start();
        UP_GAME_API void stop();

        UP_GAME_API virtual void update(float deltaTime);
        UP_GAME_API virtual void render(Renderer& renderer);

        // FIXME: find a better place for this...
        static UP_GAME_API void addRenderSystem(Space& space, Renderer& renderer);
        static UP_GAME_API void addDemoSystem(Space& space, class AudioEngine& audio);

        template <typename SystemT, typename... Args>
        void addSystem(Args&&... args) {
            UP_ASSERT(_state == State::New);
            _systems.push_back(new_box<SystemT>(*this, std::forward<Args>(args)...));
        }

        World& world() noexcept { return *_world; }

    private:
        enum class State { New, Starting, Started, Stopped };

        box<World> _world;
        vector<box<System>> _systems;
        State _state = State::New;
    };
} // namespace up
