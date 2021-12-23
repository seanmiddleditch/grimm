// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/space.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/shell/editor.h"
#include "potato/spud/hash.h"

#include <glm/vec2.hpp>

namespace up {
    class Space;
    class GpuDevice;
} // namespace up

namespace up::shell {
    class GameEditor : public Editor {
    public:
        explicit GameEditor(box<Space> space) : Editor("GameEditor"_zsv), _space(std::move(space)) { }

        zstring_view displayName() const override { return "Game"_zsv; }
        zstring_view editorClass() const override { return "potato.editor.game"_zsv; }
        EditorId uniqueId() const override { return hash_value(this); }

    protected:
        void configure() override;
        void content() override;
        void tick(float deltaTime) override;
        void render(Renderer& renderer, float deltaTime) override;
        bool hasMenu() override { return true; }

    private:
        void _resize(GpuDevice& device, glm::ivec2 size);

        box<Space> _space;
        rc<GpuResource> _buffer;
        box<GpuResourceView> _bufferView;
        EntityId _cameraId = EntityId::None;
        glm::ivec2 _viewDimensions = {0, 0};
        bool _isInputBound = false;
        bool _paused = false;
    };

    auto createGameEditor(box<Space> space) -> box<Editor>;
} // namespace up::shell
