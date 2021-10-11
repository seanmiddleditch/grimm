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

        void editObjectRaw(reflex::Schema const& schema, void* object) { _editProperties(schema, object); }

        template <typename T>
        void editObject(T& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

    private:
        bool _beginProperty(reflex::SchemaField const& field, void* object);
        void _endProperty();

        void _editProperties(reflex::Schema const& schema, void* object);
        void _editProperty(reflex::SchemaField const& field, void* object);

        void _editField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);
        void _drawObjectEditor(reflex::Schema const& schema, void* object);
        void _editArrayField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);

        void _editIntegerField(reflex::SchemaField const& field, int& value) noexcept;
        template <integral T>
        void _editIntegerField(reflex::SchemaField const& field, T& value) noexcept {
            int tmp = static_cast<int>(value);
            _editIntegerField(field, tmp);
            value = static_cast<T>(tmp);
        }
        void _editFloatField(reflex::SchemaField const& field, float& value) noexcept;
        void _editFloatField(reflex::SchemaField const& field, double& value) noexcept;
        void _editVec3Field(reflex::SchemaField const& field, glm::vec3& value) noexcept;
        void _editMat4x4Field(reflex::SchemaField const& field, glm::mat4x4& value) noexcept;
        void _editQuatField(reflex::SchemaField const& field, glm::quat& value) noexcept;
        void _editStringField(reflex::SchemaField const& field, string& value) noexcept;
        void _editAssetField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object);
        void _editUuidField(reflex::SchemaField const& field, UUID& value) noexcept;

        AssetLoader* _assetLoader = nullptr;
    };
} // namespace up::inline editor
