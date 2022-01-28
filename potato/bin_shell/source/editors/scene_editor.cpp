// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_editor.h"

#include "potato/audio/audio_engine.h"
#include "potato/audio/sound_resource.h"
#include "potato/editor/imgui_ext.h"
#include "potato/game/components/camera_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/space.h"
#include "potato/reflex/serialize.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/renderer.h"
#include "potato/shell/editor.h"
#include "potato/shell/scene_doc.h"
#include "potato/shell/selection.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/filesystem.h"
#include "potato/spud/delegate.h"
#include "potato/spud/erase.h"

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        class SceneEditorFactory : public EditorFactory {
        public:
            SceneEditorFactory(
                SceneDatabase& database,
                AssetLoader& assetLoader,
                SceneEditor::HandlePlayClicked onPlayClicked)
                : _database(database)
                , _assetLoader(assetLoader)
                , _onPlayClicked(std::move(onPlayClicked)) { }

            zstring_view editorName() const noexcept override { return SceneEditor::editorName; }

            box<Editor> createEditor() override { return nullptr; }

            box<Editor> createEditorForDocument(zstring_view filename) override {
                auto space = new_box<Space>();
                space->start();
                auto doc = new_box<SceneDocument>(string(filename), _database);

#if 0
                auto material = _assetLoader.loadAssetSync<Material>(
                    _assetLoader.translate(UUID::fromString("1fe16c8f-6225-f246-9df4-824e34a28913")));
                if (!material.isSet()) {
                    return nullptr;
                }

                auto mesh = _assetLoader.loadAssetSync<Mesh>(
                    _assetLoader.translate(UUID::fromString("8b589d73-8596-6a45-962b-4816b33d9ca3")));
                if (!mesh.isSet()) {
                    return nullptr;
                }

                auto ding = _assetLoader.loadAssetSync<SoundResource>(
                    _assetLoader.translate(UUID::fromString("df3a7c47-d06a-cc44-abc4-e928aa4ab035")));
                if (!ding.isSet()) {
                    return nullptr;
                }

                doc->createTestObjects(mesh, material, ding);
#else
                if (auto [rs, text] = fs::readText(filename); rs == IOResult::Success) {
                    nlohmann::json jsonDoc = nlohmann::json::parse(text);
                    doc->fromJson(jsonDoc, _assetLoader);
                }
#endif

                return new_box<SceneEditor>(
                    std::move(doc),
                    std::move(space),
                    _database,
                    _assetLoader,
                    [this](SceneDocument const& doc) { _onPlayClicked(doc); });
            }

        private:
            SceneDatabase& _database;
            AssetLoader& _assetLoader;
            SceneEditor::HandlePlayClicked _onPlayClicked;
        };
    } // namespace

    SceneEditor::SceneEditor(
        box<SceneDocument> sceneDoc,
        box<Space> previewScene,
        SceneDatabase& database,
        AssetLoader& assetLoader,
        HandlePlayClicked onPlayClicked)
        : Editor("SceneEditor"_zsv)
        , _previewScene(std::move(previewScene))
        , _doc(std::move(sceneDoc))
        , _onPlayClicked(std::move(onPlayClicked))
        , _database(database)
        , _assetLoader(assetLoader) {
        UP_ASSERT(_onPlayClicked);
        _arcball.target = {0, 0, 0};
        _arcball.boomLength = 40.f;
        _arcball.pitch = -glm::quarter_pi<float>();

        addPanel("Inspector", PanelDir::Right, [this] { _inspector(); });
        addPanel("Hierarchy", PanelDir::Left, [this] { _hierarchy(); });

        addAction(
            {.name = "potato.editors.scene.actions.play",
             .command = "Play Scene",
             .menu = "Actions\\Play",
             .enabled = [this] { return isActive(); },
             .action =
                 [this]() {
                     _onPlayClicked(*_doc);
                 }});
        addAction(
            {.name = "potato.editors.scene.options.grid.toggle",
             .command = "Toggle Grid",
             .menu = "View\\Options\\Grid",
             .enabled = [this] { return isActive(); },
             .checked = [this] { return _enableGrid; },
             .action =
                 [this]() {
                     _enableGrid = !_enableGrid;
                 }});
    }

    auto SceneEditor::createFactory(
        SceneDatabase& database,
        AssetLoader& assetLoader,
        SceneEditor::HandlePlayClicked onPlayClicked) -> box<EditorFactory> {
        return new_box<SceneEditorFactory>(database, assetLoader, std::move(onPlayClicked));
    }

    void SceneEditor::tick(float deltaTime) {
        _doc->syncPreview(*_previewScene);

        _previewScene->update(deltaTime);
    }

    void SceneEditor::content() {
        auto& io = ImGui::GetIO();

        ImGui::BeginGroup();
        if (ImGui::IconButton("Play", ICON_FA_PLAY)) {
            _onPlayClicked(*_doc);
        }
        ImGui::SameLine();
        if (ImGui::IconButton("Save", ICON_FA_SAVE)) {
            _save();
        }
        ImGui::EndGroup();

        auto const contentSize = ImGui::GetContentRegionAvail();
        if (contentSize.x <= 0 || contentSize.y <= 0) {
            return;
        }

        {
            _sceneDimensions = {contentSize.x, contentSize.y};

            glm::vec3 movement = {0, 0, 0};
            glm::vec3 motion = {0, 0, 0};

            auto const pos = ImGui::GetCursorScreenPos();
            if (_bufferView != nullptr) {
                ImGui::Image(_bufferView->getImguiTexture(), contentSize);
            }

            ImRect area{pos, pos + contentSize};

            auto const id = ImGui::GetID("SceneControl");
            ImGui::ItemAdd(area, id);
            ImGui::ButtonBehavior(
                area,
                id,
                nullptr,
                nullptr,
                (int)ImGuiButtonFlags_PressedOnClick | (int)ImGuiButtonFlags_MouseButtonRight |
                    (int)ImGuiButtonFlags_MouseButtonMiddle);
            ImGui::SetItemUsingMouseWheel();
            if (ImGui::IsItemActive()) {
                ImGui::CaptureMouseFromApp();

                movement = {
                    static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_D)) -
                        static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_A)),
                    static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_SPACE)) -
                        static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_C)),
                    static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_W)) -
                        static_cast<int>(ImGui::IsKeyDown(SDL_SCANCODE_S))};

                if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    motion.x = io.MouseDelta.x / contentSize.x * 2;
                    motion.y = io.MouseDelta.y / contentSize.y * 2;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                    movement.x = io.MouseDelta.x;
                    movement.y = io.MouseDelta.y;
                }
            }

            if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
                motion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;
            }

            _arcball.handleInput(movement, motion, io.DeltaTime);
        }
    }

    void SceneEditor::render(Renderer& renderer, float deltaTime) {
        if (_sceneDimensions.x == 0 || _sceneDimensions.y == 0) {
            return;
        }

        glm::ivec2 bufferSize = _buffer != nullptr ? _buffer->dimensions() : glm::vec2{0, 0};
        if (bufferSize.x != _sceneDimensions.x || bufferSize.y != _sceneDimensions.y) {
            _resize(renderer.device(), _sceneDimensions);
        }

        if (_cameraId == EntityId::None) {
            _cameraId = _previewScene->entities().createEntity();
            _previewScene->entities().addComponent<CameraComponent>(_cameraId);
            _previewScene->entities().addComponent<TransformComponent>(_cameraId);
        }

        if (auto* const cameraTrans = _previewScene->entities().getComponentSlow<TransformComponent>(_cameraId);
            cameraTrans != nullptr) {
            _arcball.applyToTransform(*cameraTrans);
        }

        if (_buffer != nullptr) {
            renderer.beginFrame();
            auto ctx = renderer.context();

            ctx.bindBackBuffer(_buffer);
            if (_enableGrid) {
                _drawGrid();
            }
            _previewScene->render(ctx);

            renderer.renderDebugDraw(ctx.commandList());

            ctx.finish();
        }
    }

    void SceneEditor::_drawGrid() {
        auto constexpr guidelines = 10;

        // The real intent here is to keep the grid roughly the same spacing in
        // pixels on the screen; this doesn't really accomplish that, though.
        // Improvements welcome.
        //
        TransformComponent const* const cameraTrans =
            _previewScene->entities().getComponentSlow<TransformComponent>(_cameraId);
        if (cameraTrans == nullptr) {
            return;
        }

        auto const cameraPos = cameraTrans->position;
        auto const logDist = std::log2(std::abs(cameraPos.y));
        auto const spacing = std::max(1, static_cast<int>(logDist) - 3);

        auto const guideSpacing = static_cast<float>(guidelines * spacing);
        float x = std::trunc(cameraPos.x / guideSpacing) * guideSpacing;
        float z = std::trunc(cameraPos.z / guideSpacing) * guideSpacing;

        DebugDrawGrid grid;
        grid.axis2 = {0, 0, 1};
        grid.offset = {x, 0, z};
        grid.halfWidth = 1000;
        grid.spacing = spacing;
        grid.guidelineSpacing = guidelines;
        drawDebugGrid(grid);
    }

    void SceneEditor::_resize(GpuDevice& device, glm::ivec2 size) {
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.bind = GpuBindFlags::ShaderResource | GpuBindFlags::RenderTarget;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = device.createTexture2D(desc, {});

        _bufferView = device.createShaderResourceView(_buffer.get());
    }

    void SceneEditor::_inspector() {
        if (_selection.empty()) {
            return;
        }

        SceneEntityId const selectedId = static_cast<SceneEntityId>(_selection.selected().front());

        int const index = _doc->indexOf(selectedId);
        if (index == -1) {
            return;
        }

        {
            char buffer[128];
            nanofmt::format_to(buffer, "{}", _doc->entityAt(index).name);
            if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                _doc->entityAt(index).name = string{buffer};
            }
        }

        if (!ImGui::BeginTable(
                "##inspector_table",
                2,
                ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize |
                    ImGuiTableFlags_SizingStretchProp)) {
            return;
        }

        ImGuiID const addComponentId = ImGui::GetID("##add_component_list");

        _propertyGrid.bindResourceLoader(&_assetLoader);

        SceneEntity& entity = _doc->entityAt(index);
        for (auto& component : entity.components) {
            ImGui::PushID(component.get());

            const bool open = _propertyGrid.beginItem(component->name.c_str());

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                ImGui::OpenPopup("##component_context_menu");
            }

            if (ImGui::BeginPopupContextItem("##component_context_menu")) {
                if (ImGui::MenuItemEx("Add", ICON_FA_PLUS_CIRCLE)) {
                    ImGui::OpenPopupEx(addComponentId);
                }
                if (ImGui::MenuItemEx("Remove", ICON_FA_TRASH)) {
                    component = nullptr;
                }
                ImGui::EndPopup();
            }

            if (open && component != nullptr) {
                if (_propertyGrid.editObjectRaw(*component->info->typeInfo().schema, component->data.get())) {
                    component->state = SceneComponent::State::Pending;
                }
                _propertyGrid.endItem();
            }

            ImGui::PopID();
        }

        ImGui::EndTable();

        erase(entity.components, nullptr);

        if (ImGui::IconButton("Add Component", ICON_FA_PLUS_CIRCLE)) {
            ImGui::OpenPopupEx(addComponentId);
        }

        if (ImGui::BeginPopupEx(
                addComponentId,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoMove)) {
            for (EditComponent const& component : _database.components()) {
                if (ImGui::MenuItem(component.name().c_str())) {
                    _doc->addNewComponent(selectedId, component);
                }
            }
            ImGui::EndPopup();
        }
    }

    void SceneEditor::_hierarchy() {
        unsigned const flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
        bool const open = ImGui::TreeNodeEx("Scene", flags);
        _hierarchyContext(SceneEntityId::None);
        if (open) {
            for (int index : _doc->indices()) {
                if (_doc->entityAt(index).parent == -1) {
                    _hierarchyShowIndex(index);
                }
            }
            ImGui::TreePop();
        }

        if (_create) {
            _create = false;
            _doc->createEntity("New Entity", _targetId);
            _targetId = SceneEntityId::None;
        }
        if (_delete) {
            _delete = false;
            _doc->deleteEntity(_targetId);
            _targetId = SceneEntityId::None;
        }
    }

    void SceneEditor::_hierarchyShowIndex(int index) {
        char label[128];

        SceneEntity const& ent = _doc->entityAt(index);

        nanofmt::format_to(label, "{} (#{})", !ent.name.empty() ? ent.name.c_str() : "Entity", ent.sceneId);

        bool const hasChildren = ent.firstChild != -1;
        bool const selected = _selection.selected(to_underlying(ent.sceneId));

        int flags =
            ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        if (ent.parent == -1) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        if (!hasChildren) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::PushID(index);
        bool const open = ImGui::TreeNodeEx(label, flags);
        if (ImGui::IsItemClicked()) {
            _selection.click(to_underlying(ent.sceneId), ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl));
        }
        _hierarchyContext(ent.sceneId);

        if (selected) {
            ImGui::SetItemDefaultFocus();
        }

        if (open) {
            for (int childIndex = ent.firstChild; childIndex != -1;
                 childIndex = _doc->entityAt(childIndex).nextSibling) {
                _hierarchyShowIndex(childIndex);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void SceneEditor::_hierarchyContext(SceneEntityId id) {
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItemEx("New Entity", ICON_FA_PLUS)) {
                _targetId = id;
                _create = true;
            }
            if (ImGui::MenuItemEx("Delete", ICON_FA_TRASH, nullptr, false, id != SceneEntityId::None)) {
                _targetId = id;
                _delete = true;
            }
            ImGui::EndPopup();
        }
    }

    void SceneEditor::_save() {
        nlohmann::json doc;
        _doc->toJson(doc);
        auto text = doc.dump(4);
        (void)fs::writeAllText(_doc->filename(), text);
    }
} // namespace up::shell
