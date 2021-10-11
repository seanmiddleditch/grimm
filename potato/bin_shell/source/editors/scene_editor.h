// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"
#include "scene_doc.h"
#include "selection.h"

#include "potato/editor/property_grid.h"
#include "potato/render/camera.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/spud/delegate.h"

#include <glm/glm.hpp>

namespace up {
    class Universe;
    class AssetLoader;
} // namespace up

namespace up::shell {
    class SceneEditor : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.scene"_zsv;

        using EnumerateComponents = delegate<view<reflex::TypeInfo const*>()>;
        using HandlePlayClicked = delegate<void(rc<Scene>)>;

        static auto createFactory(
            AudioEngine& audioEngine,
            Universe& universe,
            AssetLoader& assetLoader,
            SceneEditor::EnumerateComponents components,
            SceneEditor::HandlePlayClicked onPlayClicked) -> box<EditorFactory>;

        explicit SceneEditor(
            box<SceneDocument> sceneDoc,
            AssetLoader& assetLoader,
            EnumerateComponents& components,
            HandlePlayClicked& onPlayClicked)
            : Editor("SceneEditor"_zsv)
            , _doc(std::move(sceneDoc))
            , _cameraController(_camera)
            , _components(std::move(components))
            , _onPlayClicked(std::move(onPlayClicked))
            , _assetLoader(assetLoader) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }

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
        void _hierarchyContext(EntityId id);
        void _save();

        rc<GpuTexture> _buffer;
        box<SceneDocument> _doc;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        ArcBallCameraController _cameraController;
        SelectionState _selection;
        EnumerateComponents _components;
        PropertyGrid _propertyGrid;
        HandlePlayClicked _onPlayClicked;
        glm::ivec2 _sceneDimensions = {0, 0};
        bool _enableGrid = true;
        bool _create = false;
        bool _delete = false;
        EntityId _targetId = EntityId::None;
        AssetLoader& _assetLoader;
    };
} // namespace up::shell
