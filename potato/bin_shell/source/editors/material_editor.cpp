// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "material_editor.h"

#include "potato/editor/editor_manager.h"
#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
#include "potato/reflex/serialize.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>

namespace up::shell {
    namespace {
        class MaterialEditorFactory : public EditorFactory<MaterialEditor> {
        public:
            MaterialEditorFactory(PropertyGrid& propertyGrid, AssetLoader& assetLoader) noexcept
                : _propertyGrid(propertyGrid)
                , _assetLoader(assetLoader) { }

            box<EditorBase> createEditor(EditorParams const& params) override {
                if (auto [rs, text] = fs::readText(params.documentPath); rs == IOResult::Success) {
                    nlohmann::json jsonDoc = nlohmann::json::parse(text);
                    auto material = new_box<schema::Material>();
                    if (reflex::decodeFromJson(jsonDoc, *material)) {
                        return new_box<MaterialEditor>(
                            params,
                            _propertyGrid,
                            _assetLoader,
                            std::move(material),
                            string(params.documentPath));
                    }
                }
                return nullptr;
            }

        private:
            PropertyGrid& _propertyGrid;
            AssetLoader& _assetLoader;
        };
    } // namespace
} // namespace up::shell

up::shell::MaterialEditor::MaterialEditor(
    EditorParams const& params,
    PropertyGrid& propertyGrid,
    AssetLoader& assetLoader,
    box<schema::Material> material,
    string filename)
    : Editor(params)
    , _assetLoader(assetLoader)
    , _material(std::move(material))
    , _filename(std::move(filename))
    , _propertyGrid(propertyGrid) { }

void up::shell::MaterialEditor::addFactory(
    EditorManager& editors,
    PropertyGrid& propertyGrid,
    AssetLoader& assetLoader) {
    editors.addFactory<MaterialEditorFactory>(propertyGrid, assetLoader);
}

void up::shell::MaterialEditor::content(CommandManager&) {
    ImGui::BeginGroup();
    if (ImGui::IconButton("Save", ICON_FA_SAVE)) {
        _save();
    }
    ImGui::EndGroup();

    if (!ImGui::BeginTable(
            "##material",
            2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize |
                ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    _propertyGrid.editObject(*_material);

    ImGui::EndTable();
}

void up::shell::MaterialEditor::_save() {
    nlohmann::json doc;
    reflex::encodeToJson(doc, *_material);
    auto text = doc.dump(4);
    (void)fs::writeAllText(_filename, text);
}
