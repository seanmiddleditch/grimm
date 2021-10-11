// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"
#include "material_schema.h"

#include "potato/editor/property_grid.h"

namespace up {
    class AssetLoader;
} // namespace up

namespace up::shell {
    class MaterialEditor final : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.material"_zsv;

        MaterialEditor(AssetLoader& assetLoader, box<schema::Material> material, string filename);

        static auto createFactory(AssetLoader& assetLoader) -> box<EditorFactory>;

        zstring_view displayName() const override { return "Material"_zsv; }
        zstring_view editorClass() const override { return editorName; }
        EditorId uniqueId() const override { return hash_value(this); }

    private:
        void configure() override;
        void content() override;

        void _save();

        AssetLoader& _assetLoader;
        box<schema::Material> _material;
        string _filename;
        PropertyGrid _propertyGrid;
    };
} // namespace up::shell
