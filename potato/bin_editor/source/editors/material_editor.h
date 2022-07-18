// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/editor.h"
#include "potato/editor/property_grid.h"
#include "potato/schema/material_schema.h"

namespace up {
    class AssetLoader;
    class PropertyGrid;
} // namespace up

namespace up::shell {
    class MaterialEditor final : public Editor<MaterialEditor> {
    public:
        static constexpr EditorTypeId editorTypeId{"potato.editor.material"};

        MaterialEditor(
            EditorParams const& params,
            PropertyGrid& propertyGrid,
            box<schema::Material> material,
            string filename);

        static void addFactory(Workspace& workspace, PropertyGrid& propertyGrid);

        zstring_view displayName() const override { return "Material"_zsv; }
        zstring_view documentPath() const override { return _filename; }

    private:
        void content(CommandManager&) override;

        void _save();

        box<schema::Material> _material;
        string _filename;
        PropertyGrid& _propertyGrid;
    };
} // namespace up::shell
