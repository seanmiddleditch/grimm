// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/game/space.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/shell/camera_controller.h"
#include "potato/shell/editor.h"
#include "potato/spud/hash.h"

namespace up {
    class Space;
    class GpuDevice;
}

namespace up::shell {
    class GameEditor : public Editor {
    public:
        explicit GameEditor(box<Space> space)
            : Editor("GameEditor"_zsv)
            , _space(std::move(space))
            , _cameraController(_camera) {
            _camera.position = {0, 10, 15};
            _camera.lookAt({0, 0, 0});
        }

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
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        Transform _camera;
        FlyCameraController _cameraController;
        EntityId _cameraId = EntityId::None;
        glm::ivec2 _viewDimensions = {0, 0};
        bool _isInputBound = false;
        bool _paused = false;
    };

    auto createGameEditor(box<Space> space) -> box<Editor>;
} // namespace up::shell
