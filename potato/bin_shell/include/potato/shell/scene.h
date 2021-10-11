// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
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

    class Scene : public shared<Scene> {
    public:
        explicit Scene(Universe& universe, AudioEngine& audioEngine);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void tick(float frameTime);
        void flush();
        void render(RenderContext& ctx);

        bool load(Stream file);
        void save(Stream file);

        bool playing() const { return _playing; }
        bool playing(bool active) { return _playing = active; }

        World& world() noexcept { return _world; }
        Universe& universe() noexcept { return _universe; }

    private:
        AudioEngine& _audioEngine;
        Universe& _universe;
        World _world;
        bool _playing = false;

        Query<components::Transform, components::Wave> _waveQuery;
        Query<components::Transform> _orbitQuery;
        Query<components::Transform, components::Spin> _spinQuery;
        Query<components::Ding> _dingQuery;
        Query<components::Transform> _transformQuery;
        Query<components::Mesh, components::Transform> _renderableMeshQuery;
    };
} // namespace up
