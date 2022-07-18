// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "material_editor.h"

#include "potato/editor/workspace.h"
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
            explicit MaterialEditorFactory(PropertyGrid& propertyGrid) noexcept : _propertyGrid(propertyGrid) { }

            box<EditorBase> createEditor(EditorParams const& params) override {
                if (auto [rs, text] = fs::readText(params.documentPath); rs == IOResult::Success) {
                    nlohmann::json jsonDoc = nlohmann::json::parse(text);
                    auto material = new_box<schema::Material>();
                    if (reflex::decodeFromJson(jsonDoc, *material)) {
                        return new_box<MaterialEditor>(
                            params,
                            _propertyGrid,
                            std::move(material),
                            string(params.documentPath));
                    }
                }
                return nullptr;
            }

        private:
            PropertyGrid& _propertyGrid;
        };
    } // namespace
} // namespace up::shell

up::shell::MaterialEditor::MaterialEditor(
    EditorParams const& params,
    PropertyGrid& propertyGrid,
    box<schema::Material> material,
    string filename)
    : Editor(params)
    , _material(std::move(material))
    , _filename(std::move(filename))
    , _propertyGrid(propertyGrid) { }

void up::shell::MaterialEditor::addFactory(Workspace& workspace, PropertyGrid& propertyGrid) {
    workspace.addFactory<MaterialEditorFactory>(propertyGrid);
}

void up::shell::MaterialEditor::content(CommandManager&) {
    ImGui::BeginGroup();
    if (ImGui::IconButton("Save", ICON_FA_SAVE)) {
        _save();
    }
    ImGui::EndGroup();

    if (_propertyGrid.beginTable()) {
        _propertyGrid.editObject(*_material);
        _propertyGrid.endTable();
    }
}

void up::shell::MaterialEditor::_save() {
    nlohmann::json doc;
    reflex::encodeToJson(doc, *_material);
    auto text = doc.dump(4);
    (void)fs::writeAllText(_filename, text);
}
