// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "camera.h"
#include "camera_controller.h"
#include "editor.h"

#include "potato/game/space.h"
#include "potato/render/camera.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"

namespace up {
    class Space;
}

namespace up::shell {
    class GameEditor : public Editor {
    public:
        explicit GameEditor(box<Space> space)
            : Editor("GameEditor"_zsv)
            , _space(std::move(space))
            , _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }

        zstring_view displayName() const override { return "Game"; }
        zstring_view editorClass() const override { return "potato.editor.game"; }
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
        box<RenderCamera> _renderCamera;
        Camera _camera;
        FlyCameraController _cameraController;
        glm::ivec2 _viewDimensions = {0, 0};
        bool _isInputBound = false;
        bool _paused = false;
    };

    auto createGameEditor(box<Space> space) -> box<Editor>;
} // namespace up::shell
