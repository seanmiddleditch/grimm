// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/property_grid.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/shell/camera.h"
#include "potato/shell/camera_controller.h"
#include "potato/shell/editor.h"
#include "potato/shell/scene_doc.h"
#include "potato/shell/selection.h"
#include "potato/spud/delegate.h"

#include <glm/glm.hpp>

namespace up {
    class AssetLoader;
} // namespace up

namespace up::shell {
    class SceneEditor : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.scene"_zsv;

        using HandlePlayClicked = delegate<void(SceneDocument const& doc)>;

        static auto createFactory(
            SceneDatabase& database,
            AssetLoader& assetLoader,
            SceneEditor::HandlePlayClicked onPlayClicked) -> box<EditorFactory>;

        explicit SceneEditor(
            box<SceneDocument> sceneDoc,
            box<Space> previewScene,
            SceneDatabase& database,
            AssetLoader& assetLoader,
            HandlePlayClicked onPlayClicked);

        zstring_view displayName() const override { return "Scene"_zsv; }
        zstring_view editorClass() const override { return editorName; }
        EditorId uniqueId() const override { return hash_value(this); }

        void tick(float deltaTime) override;

    protected:
        void configure() override;
        void content() override;
        void render(Renderer& renderer, float deltaTime) override;

    private:
        void _drawGrid();
        void _resize(GpuDevice& device, glm::ivec2 size);
        void _inspector();
        void _hierarchy();
        void _hierarchyShowIndex(int index);
        void _hierarchyContext(SceneEntityId id);
        void _save();

        rc<GpuTexture> _buffer;
        box<Space> _previewScene;
        box<SceneDocument> _doc;
        box<GpuResourceView> _bufferView;
        Camera _camera;
        ArcBallCameraController _cameraController;
        SelectionState _selection;
        PropertyGrid _propertyGrid;
        HandlePlayClicked _onPlayClicked;
        glm::ivec2 _sceneDimensions = {0, 0};
        bool _enableGrid = true;
        bool _create = false;
        bool _delete = false;
        SceneEntityId _targetId = SceneEntityId::None;
        EntityId _cameraId = EntityId::None;
        SceneDatabase& _database;
        AssetLoader& _assetLoader;
    };
} // namespace up::shell
