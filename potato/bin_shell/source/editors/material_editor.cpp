// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "material_editor.h"

#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
#include "potato/reflex/serialize.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>

namespace up::shell {
    namespace {
        class MaterialEditorFactory : public EditorFactory {
        public:
            MaterialEditorFactory(AssetLoader& assetLoader) : _assetLoader(assetLoader) {}

            zstring_view editorName() const noexcept override { return MaterialEditor::editorName; }

            box<Editor> createEditorForDocument(zstring_view filename) override {
                if (auto [rs, text] = fs::readText(filename); rs == IOResult::Success) {
                    nlohmann::json jsonDoc = nlohmann::json::parse(text);
                    auto material = new_box<schema::Material>();
                    if (reflex::decodeFromJson(jsonDoc, *material)) {
                        return new_box<MaterialEditor>(_assetLoader, std::move(material), string(filename));
                    }
                }
                return nullptr;
            }

            box<Editor> createEditor() override { return nullptr; }

        private:
            AssetLoader& _assetLoader;
        };
    } // namespace
} // namespace up::shell

up::shell::MaterialEditor::MaterialEditor(AssetLoader& assetLoader, box<schema::Material> material, string filename)
    : Editor(editorName)
    , _assetLoader(assetLoader)
    , _material(std::move(material))
    , _filename(std::move(filename)) {}

auto up::shell::MaterialEditor::createFactory(AssetLoader& assetLoader) -> box<EditorFactory> {
    return new_box<MaterialEditorFactory>(assetLoader);
}

void up::shell::MaterialEditor::configure() {
    _propertyGrid.bindResourceLoader(&_assetLoader);
}

void up::shell::MaterialEditor::content() {
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
