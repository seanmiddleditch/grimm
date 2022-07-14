// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/editor.h"
#include "potato/editor/property_grid.h"
#include "potato/game/arcball.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/shell/scene_doc.h"
#include "potato/shell/selection.h"

#include <glm/glm.hpp>

namespace up {
    class AssetLoader;
} // namespace up

namespace up::shell {
    class SceneEditor : public Editor<SceneEditor> {
    public:
        static constexpr EditorTypeId editorTypeId{"potato.editor.scene"};

        static void addFactory(
            Workspace& workspace,
            SceneDatabase& database,
            PropertyGrid& propertyGrid,
            AssetLoader& assetLoader);
        static void addCommands(CommandManager& commands);

        explicit SceneEditor(
            EditorParams const& params,
            box<SceneDocument> sceneDoc,
            box<Space> previewScene,
            SceneDatabase& database,
            PropertyGrid& propertyGrid);

        zstring_view displayName() const override { return "Scene"_zsv; }
        zstring_view documentPath() const override { return _doc->filename(); }

        void tick(float deltaTime) override;

        SceneDocument& document() noexcept { return *_doc; }

        bool hasGrid() const noexcept { return _enableGrid; }
        void toggleGrid() noexcept { _enableGrid = !_enableGrid; }

    protected:
        void content(CommandManager&) override;
        void render(Renderer& renderer, float deltaTime) override;

    private:
        struct PlayCommandHandler;
        struct ToggleGridHandler;

        void _drawGrid();
        void _resize(GpuDevice& device, glm::ivec2 size);
        void _inspector();
        void _hierarchy();
        void _hierarchyShowIndex(int index);
        void _hierarchyContext(SceneEntityId id);
        void _save();

        rc<GpuResource> _buffer;
        box<Space> _previewScene;
        box<SceneDocument> _doc;
        box<GpuResourceView> _bufferView;
        ArcBall _arcball;
        SelectionState _selection;
        glm::ivec2 _sceneDimensions = {0, 0};
        bool _enableGrid = true;
        bool _create = false;
        bool _delete = false;
        SceneEntityId _targetId = SceneEntityId::None;
        EntityId _cameraId = EntityId::None;
        SceneDatabase& _database;
        PropertyGrid& _propertyGrid;
    };
} // namespace up::shell
