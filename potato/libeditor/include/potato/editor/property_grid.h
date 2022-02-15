// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <glm/fwd.hpp>

namespace up {
    class AssetLoader;
    class PropertyEditor;
    class PropertyGrid;
    class UUID;

    enum class PropertyExpandable { None, Empty, Children };

    struct PropertyItemInfo {
        struct ArrayOps {
            bool canRemove = false; // if a remove icon should be present
            bool canMove = false; // if drag-and-drop move should be present
            bool wantRemove = false; // true if the item should be removed
            int moveFromIndex = -1; // >= 0 to replace with item at given index
        };

        reflex::Schema const& schema;
        void* object = nullptr;
        reflex::SchemaField const* field = nullptr; // either this or index are required
        int index = -1; // for arrays
        ArrayOps* arrayOps = nullptr;
    };

    class PropertyEditor {
    public:
        virtual ~PropertyEditor() = default;

        virtual bool edit(PropertyItemInfo const& info) = 0;

        virtual PropertyExpandable expandable(PropertyItemInfo const& info) { return PropertyExpandable::None; }
        virtual bool children(PropertyGrid& propertyGrid, PropertyItemInfo const& info) { return false; }

        virtual bool canAddItem(PropertyItemInfo const& info) { return false; }
        virtual void addItem(PropertyItemInfo const& info) { }
    };

    class PropertyGrid {
    public:
        explicit PropertyGrid(AssetLoader& assetLoader) noexcept;

        bool beginTable(char const* label = nullptr);
        void endTable();

        bool editObjectRaw(reflex::Schema const& schema, void* object);
        bool editItem(PropertyItemInfo const& info);

        template <typename T>
        void editObject(T& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

        void addPropertyEditor(reflex::SchemaId schemaId, box<PropertyEditor> editor);

        PropertyEditor* findPropertyEditor(reflex::Schema const& schema) const noexcept;

    private:
        bool _editInternal(PropertyEditor& propertyEditor, PropertyItemInfo const& info);
        bool _showLabel(PropertyItemInfo const& info) noexcept;

        vector<box<PropertyEditor>> _propertyEditors;
        hash_map<reflex::SchemaPrimitive, uint32> _primitiveEditorMap;
        hash_map<reflex::SchemaId, uint32> _typedEditorMap;
    };
} // namespace up
