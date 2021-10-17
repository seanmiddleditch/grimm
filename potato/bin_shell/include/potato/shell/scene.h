// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/query.h"
#include "potato/game/space.h"
#include "potato/game/universe.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"

namespace up::components {
    struct Transform;
    struct Mesh;
    struct Wave;
    struct Spin;
    struct Ding;
} // namespace up::components

namespace up {
    class RenderContext;
    class AudioEngine;

    class Scene : public Space {
    public:
        explicit Scene(Universe& universe, AudioEngine& audioEngine);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void update(float frameTime) override;

        bool load(Stream file);
        void save(Stream file);

        bool playing() const { return _playing; }
        bool playing(bool active) { return _playing = active; }

        Universe& universe() noexcept { return _universe; }

    private:
        AudioEngine& _audioEngine;
        Universe& _universe;
        bool _playing = false;

        Query<components::Transform, components::Wave> _waveQuery;
        Query<components::Transform> _orbitQuery;
        Query<components::Transform, components::Spin> _spinQuery;
        Query<components::Ding> _dingQuery;
    };
} // namespace up
