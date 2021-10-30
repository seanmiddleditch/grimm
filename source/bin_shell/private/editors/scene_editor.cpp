// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_editor.h"
#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"
#include "scene_doc.h"
#include "selection.h"

#include "potato/audio/audio_engine.h"
#include "potato/audio/sound_resource.h"
#include "potato/editor/imgui_ext.h"
#include "potato/reflex/serialize.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/renderer.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/filesystem.h"
#include "potato/spud/delegate.h"
#include "potato/spud/fixed_string_writer.h"

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        class SceneEditorFactory : public EditorFactory {
        public:
            SceneEditorFactory(
                AudioEngine& audioEngine,
                Universe& universe,
                AssetLoader& assetLoader,
                SceneEditor::EnumerateComponents components,
                SceneEditor::HandlePlayClicked onPlayClicked)
                : _audioEngine(audioEngine)
                , _universe(universe)
                , _assetLoader(assetLoader)
                , _components(std::move(components))
                , _onPlayClicked(std::move(onPlayClicked)) {}

            zstring_view editorName() const noexcept override { return SceneEditor::editorName; }

            box<Editor> createEditor() override { return nullptr; }

            box<Editor> createEditorForDocument(zstring_view filename) override {
                auto scene = new_shared<Scene>(_universe, _audioEngine);
                auto doc = new_box<SceneDocument>(string(filename), std::move(scene));

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

                return new_box<SceneEditor>(std::move(doc), _assetLoader, _components, _onPlayClicked);
            }

        private:
            AudioEngine& _audioEngine;
            Universe& _universe;
            AssetLoader& _assetLoader;
            SceneEditor::EnumerateComponents _components;
            SceneEditor::HandlePlayClicked _onPlayClicked;
        };
    } // namespace
} // namespace up::shell

auto up::shell::SceneEditor::createFactory(
    AudioEngine& audioEngine,
    Universe& universe,
    AssetLoader& assetLoader,
    SceneEditor::EnumerateComponents components,
    SceneEditor::HandlePlayClicked onPlayClicked) -> box<EditorFactory> {
    return new_box<SceneEditorFactory>(
        audioEngine,
        universe,
        assetLoader,
        std::move(components),
        std::move(onPlayClicked));
}

void up::shell::SceneEditor::tick(float deltaTime) {
    _doc->scene()->tick(deltaTime);
    _doc->scene()->flush();
}

void up::shell::SceneEditor::configure() {
    auto const inspectorId = addPanel("Inspector", [this] { _inspector(); });
    auto const hierarchyId = addPanel("Hierarchy", [this] { _hierarchy(); });

    dockPanel(inspectorId, ImGuiDir_Right, contentId(), 0.25f);
    dockPanel(hierarchyId, ImGuiDir_Down, inspectorId, 0.65f);

    addAction(
        {.name = "potato.editors.scene.actions.play",
         .command = "Play Scene",
         .menu = "Actions\\Play",
         .enabled = [this] { return isActive(); },
         .action =
             [this]() {
                 _onPlayClicked(_doc->scene());
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

void up::shell::SceneEditor::content() {
    auto& io = ImGui::GetIO();

    auto const contentSize = ImGui::GetContentRegionAvail();

    if (contentSize.x <= 0 || contentSize.y <= 0) {
        return;
    }

    ImGui::BeginGroup();
    if (ImGui::IconButton("Play", ICON_FA_PLAY)) {
        _onPlayClicked(_doc->scene());
    }
    ImGui::SameLine();
    if (ImGui::IconButton("Save", ICON_FA_SAVE)) {
        _save();
    }
    ImGui::EndGroup();

    ImGui::BeginChild("SceneContent", contentSize, false);
    {
        _sceneDimensions = {contentSize.x, contentSize.y};

        glm::vec3 movement = {0, 0, 0};
        glm::vec3 motion = {0, 0, 0};

        auto const pos = ImGui::GetCursorScreenPos();
        if (_bufferView != nullptr) {
            ImGui::Image(_bufferView.get(), contentSize);
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
        if (ImGui::IsItemActive()) {
            ImGui::CaptureMouseFromApp();

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

        _cameraController.apply(_camera, movement, motion, io.DeltaTime);

        ImGui::EndChild();
    }
}

void up::shell::SceneRenderer::onSchedule(up::RenderContext& ctx) {}

void up::shell::SceneRenderer::onRender(up::RenderContext& ctx) {
    _owner->_render(ctx, _deltaTime);
}

void up::shell::SceneEditor::render(Renderer& renderer, float deltaTime) {
    if (_sceneDimensions.x == 0 || _sceneDimensions.y == 0) {
        return;
    }

    glm::ivec2 bufferSize = _buffer != nullptr ? _buffer->dimensions() : glm::vec2{0, 0};
    if (bufferSize.x != _sceneDimensions.x || bufferSize.y != _sceneDimensions.y) {
        _resize(renderer.device(), _sceneDimensions);
    }
    
    if (_renderCamera == nullptr) {
        _renderCamera = new_box<RenderCamera>();
    }

    if (_sceneRenderer == nullptr) {
        _sceneRenderer = new_box<SceneRenderer>(this);
    }

    renderer.createRendarable(_sceneRenderer.get());

    if (_doc->scene() != nullptr) {
        _doc->scene()->render(renderer);
    }
}

void up::shell::SceneEditor::_render(up::RenderContext& ctx, float deltaTime) {

    if (_buffer != nullptr) {
        
         _renderCamera->resetBackBuffer(_buffer);
        if (_enableGrid) {
            _drawGrid();
        }

        _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());

    }
}

void up::shell::SceneEditor::_drawGrid() {
    auto constexpr guidelines = 10;

    // The real intent here is to keep the grid roughly the same spacing in
    // pixels on the screen; this doesn't really accomplish that, though.
    // Improvements welcome.
    //
    auto const cameraPos = _camera.position();
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

void up::shell::SceneEditor::_resize(GpuDevice& device, glm::ivec2 size) {
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _buffer = device.createTexture2D(desc, {});

    //_bufferView = device.createShaderResourceView(_buffer.get());
}

void up::shell::SceneEditor::_inspector() {
    ComponentId deletedComponent = ComponentId::Unknown;

    if (_doc->scene() == nullptr) {
        return;
    }

    if (_selection.empty()) {
        return;
    }

    EntityId const selectedId = static_cast<EntityId>(_selection.selected().front());

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

    _doc->scene()->world().interrogateEntityUnsafe(
        selectedId,
        [&](EntityId entity, ArchetypeId archetype, reflex::TypeInfo const* typeInfo, auto* data) {
            ImGui::PushID(data);

            const bool open = _propertyGrid.beginItem(typeInfo->name.c_str());

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                ImGui::OpenPopup("##component_context_menu");
            }

            if (ImGui::BeginPopupContextItem("##component_context_menu")) {
                if (ImGui::IconMenuItem("Add", ICON_FA_PLUS_CIRCLE)) {
                    ImGui::OpenPopupEx(addComponentId);
                }
                if (ImGui::IconMenuItem("Remove", ICON_FA_TRASH)) {
                    deletedComponent = static_cast<ComponentId>(typeInfo->hash);
                }
                ImGui::EndPopup();
            }

            if (open) {
                if (typeInfo->schema != nullptr) {
                    _propertyGrid.editObjectRaw(*typeInfo->schema, data);
                }
                _propertyGrid.endItem();
            }

            ImGui::PopID();
        });

    ImGui::EndTable();

    if (deletedComponent != ComponentId::Unknown) {
        _doc->scene()->world().removeComponent(selectedId, deletedComponent);
    }

    if (ImGui::IconButton("Add Component", ICON_FA_PLUS_CIRCLE)) {
        ImGui::OpenPopupEx(addComponentId);
    }

    if (ImGui::BeginPopupEx(
            addComponentId,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoMove)) {
        for (reflex::TypeInfo const* typeInfo : _components()) {
            if (_doc->scene()->world().getComponentSlowUnsafe(selectedId, static_cast<ComponentId>(typeInfo->hash)) ==
                nullptr) {
                if (ImGui::IconMenuItem(typeInfo->name.c_str())) {
                    _doc->scene()->world().addComponentDefault(selectedId, *typeInfo);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void up::shell::SceneEditor::_hierarchy() {
    unsigned const flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
    bool const open = ImGui::TreeNodeEx("Scene", flags);
    _hierarchyContext(EntityId::None);
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
        _targetId = EntityId::None;
    }
    if (_delete) {
        _delete = false;
        _doc->deleteEntity(_targetId);
        _targetId = EntityId::None;
    }
}

void up::shell::SceneEditor::_hierarchyShowIndex(int index) {
    char label[128];

    SceneEntity const& ent = _doc->entityAt(index);

    nanofmt::format_to(label, "{} (#{})", !ent.name.empty() ? ent.name.c_str() : "Entity", ent.id);

    bool const hasChildren = ent.firstChild != -1;
    bool const selected = _selection.selected(to_underlying(ent.id));

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
        _selection.click(to_underlying(ent.id), ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl));
    }
    _hierarchyContext(ent.id);

    if (selected) {
        ImGui::SetItemDefaultFocus();
    }

    if (open) {
        for (int childIndex = ent.firstChild; childIndex != -1; childIndex = _doc->entityAt(childIndex).nextSibling) {
            _hierarchyShowIndex(childIndex);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void up::shell::SceneEditor::_hierarchyContext(EntityId id) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::IconMenuItem("New Entity", ICON_FA_PLUS)) {
            _targetId = id;
            _create = true;
        }
        if (ImGui::IconMenuItem("Delete", ICON_FA_TRASH, nullptr, false, id != EntityId::None)) {
            _targetId = id;
            _delete = true;
        }
        ImGui::EndPopup();
    }
}

void up::shell::SceneEditor::_save() {
    nlohmann::json doc;
    _doc->toJson(doc);
    auto text = doc.dump(4);
    (void)fs::writeAllText(_doc->filename(), text);
}
