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
    class PropertyGrid;
    class UUID;

    struct PropertyInfo {
        reflex::SchemaField const& field;
        reflex::Schema const& schema;
        void* object = nullptr;
        int index = -1; // for arrays
        bool canRemove = false; // if a remove icon should be present
    };

    class PropertyEditor {
    public:
        virtual ~PropertyEditor() = default;

        virtual bool edit(PropertyInfo const& info) = 0;
    };

    class PropertyGrid {
    public:
        explicit PropertyGrid(AssetLoader& assetLoader) noexcept;

        bool beginTable();
        void endTable();

        bool editObjectRaw(reflex::Schema const& schema, void* object);

        template <typename T>
        void editObject(T& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

        void addPropertyEditor(box<PropertyEditor> editor);

    private:
        struct ArrayOps;
        struct ItemState;

        bool _editElements(PropertyInfo const& info, ItemState& state);
        bool _editProperty(PropertyInfo const& info);

        void _label(PropertyInfo const& info, ItemState& state, ArrayOps* ops = nullptr) noexcept;
        bool _applyState(PropertyInfo const& info, ItemState const& state);
        bool _editField(PropertyInfo const& info, ItemState& state, ArrayOps* ops = nullptr);

        PropertyEditor* _selectEditor(PropertyInfo const& info) noexcept;

        vector<box<PropertyEditor>> _propertyEditors;
        hash_map<reflex::SchemaPrimitive, uint32> _primitiveEditorMap;
    };
} // namespace up
