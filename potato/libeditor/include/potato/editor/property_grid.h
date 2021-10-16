// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"
#include "potato/spud/string.h"

#include <glm/fwd.hpp>

namespace up {
    class AssetLoader;
    class UUID;
} // namespace up

namespace up::inline editor {
    class PropertyEditor {
    public:
        virtual ~PropertyEditor() = default;

        virtual void drawLabel(reflex::SchemaField const& field, void* member) = 0;
        virtual void drawInput(reflex::SchemaField const& field, void* member) = 0;
    };

    class PropertyGrid {
    public:
        void bindResourceLoader(AssetLoader* assetLoader) { _assetLoader = assetLoader; }

        bool beginItem(char const* label);
        void endItem();

        bool editObjectRaw(reflex::Schema const& schema, void* object) { return _editProperties(schema, object); }

        template <typename T>
        void editObject(T& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

    private:
        bool _beginProperty(reflex::SchemaField const& field, void* object);
        void _endProperty();

        bool _editProperties(reflex::Schema const& schema, void* object);
        bool _editProperty(reflex::SchemaField const& field, void* object);

        bool _editField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);
        bool _drawObjectEditor(reflex::Schema const& schema, void* object);
        bool _editArrayField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);

        bool _editIntegerField(reflex::SchemaField const& field, int& value) noexcept;
        template <integral T>
        bool _editIntegerField(reflex::SchemaField const& field, T& value) noexcept {
            int tmp = static_cast<int>(value);
            _editIntegerField(field, tmp);
            value = static_cast<T>(tmp);
        }
        bool _editFloatField(reflex::SchemaField const& field, float& value) noexcept;
        bool _editFloatField(reflex::SchemaField const& field, double& value) noexcept;
        bool _editVec3Field(reflex::SchemaField const& field, glm::vec3& value) noexcept;
        bool _editMat4x4Field(reflex::SchemaField const& field, glm::mat4x4& value) noexcept;
        bool _editQuatField(reflex::SchemaField const& field, glm::quat& value) noexcept;
        bool _editStringField(reflex::SchemaField const& field, string& value) noexcept;
        bool _editAssetField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);
        bool _editUuidField(reflex::SchemaField const& field, UUID& value) noexcept;

        AssetLoader* _assetLoader = nullptr;
    };
} // namespace up::inline editor
