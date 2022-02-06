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
    };

    class PropertyEditor {
    public:
        virtual ~PropertyEditor() = default;

        virtual void label(PropertyInfo const& info);
        virtual bool edit(PropertyInfo const& info) = 0;
        virtual bool expandable(PropertyInfo const& info) const { return false; }
    };

    class PropertyGrid {
    public:
        explicit PropertyGrid(AssetLoader& assetLoader) noexcept;

        bool editObjectRaw(reflex::Schema const& schema, void* object) { return _editProperties(schema, object); }

        template <typename T>
        void editObject(T& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

        void addPropertyEditor(box<PropertyEditor> editor);

    private:
        bool _editProperties(reflex::Schema const& schema, void* object);

        bool _editField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);
        bool _drawObjectEditor(reflex::Schema const& schema, void* object);
        bool _editArrayField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);

        PropertyEditor* _selectEditor(PropertyInfo const& info) noexcept;

        vector<box<PropertyEditor>> _propertyEditors;
        hash_map<reflex::SchemaPrimitive, uint32> _primitiveEditorMap;
    };
} // namespace up
