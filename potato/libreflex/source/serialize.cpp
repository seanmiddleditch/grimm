// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/reflex/serialize.h"

#include "potato/schema/common_schema.h"
#include "potato/schema/tools_schema.h"
#include "potato/runtime/assertion.h"
#include "potato/runtime/asset.h"
#include "potato/runtime/json.h"
#include "potato/spud/utility.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

namespace up::reflex::_detail {
    static bool encodeObject(nlohmann::json& json, Schema const& schema, void const* obj);
    static bool encodeArray(nlohmann::json& json, Schema const& schema, void const* arr);
    static bool encodeAssetRef(nlohmann::json& json, Schema const& schema, void const* obj);
    static bool encodeUuid(nlohmann::json& json, Schema const& schema, void const* obj);
    static bool encodeValue(nlohmann::json& json, Schema const& schema, void const* obj);

    static bool decodeObject(nlohmann::json const& json, Schema const& schema, void* obj);
    static bool decodeArray(nlohmann::json const& json, Schema const& schema, void* arr);
    static bool decodeAssetRef(nlohmann::json const& json, Schema const& schema, void* obj);
    static bool decodeUuid(nlohmann::json const& json, Schema const& schema, void* obj);
    static bool decodeValue(nlohmann::json const& json, Schema const& schema, void* obj);

    static int64 readInt(Schema const& schema, void const* obj);
    static void writeInt(Schema const& schema, void* obj, int64 value);
} // namespace up::reflex::_detail

bool up::reflex::encodeToJsonRaw(nlohmann::json& json, Schema const& schema, void const* memory) {
    UP_ASSERT(memory != nullptr);
    return _detail::encodeValue(json, schema, memory);
}

bool up::reflex::decodeFromJsonRaw(nlohmann::json const& json, Schema const& schema, void* memory) {
    UP_ASSERT(memory != nullptr);
    return _detail::decodeValue(json, schema, memory);
}

bool up::reflex::_detail::encodeObject(nlohmann::json& json, Schema const& schema, void const* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Object);

    json = nlohmann::json::object();
    json["$schema"] = schema.name;

    bool success = true;
    for (SchemaField const& field : schema.fields) {
        auto const* jsonName = queryAnnotation<schema::json>(field);
        char const* const fieldName = jsonName == nullptr ? field.name.c_str() : jsonName->name.c_str();
        nlohmann::json& sub = json[fieldName];
        success = encodeValue(sub, *field.schema, static_cast<char const*>(obj) + field.offset) && success;
    }

    return success;
}

bool up::reflex::_detail::encodeArray(nlohmann::json& json, Schema const& schema, void const* arr) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Array);

    if (schema.operations->arrayGetSize == nullptr) {
        return false;
    }
    if (schema.operations->arrayElementAt == nullptr) {
        return false;
    }

    json = nlohmann::json::array();

    bool success = true;
    size_t const size = schema.operations->arrayGetSize(arr);
    for (size_t index = 0; index != size; ++index) {
        void const* elem = schema.operations->arrayElementAt(arr, index);
        json.push_back(nlohmann::json());
        success = encodeValue(json.back(), *schema.elementType, elem) && success;
    }

    return success;
}

bool up::reflex::_detail::encodeAssetRef(nlohmann::json& json, Schema const& schema, void const* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::AssetRef);
    UP_ASSERT(schema.operations != nullptr);

    json = nlohmann::json::object();
    json["$schema"] = schema.name;

    auto const* const resourceAnnotation = queryAnnotation<schema::AssetType>(schema);
    if (resourceAnnotation != nullptr) {
        json["type"] = resourceAnnotation->assetType;
    }

    auto const* const assetHandle = static_cast<UntypedAssetHandle const*>(obj);
    AssetKey const& key = assetHandle->assetKey();
    if (key.uuid.isValid()) {
        json["uuid"] = key.uuid.toString();
        if (!key.logical.empty()) {
            json["logical"] = key.logical;
        }
    }
    else if (assetHandle->isSet()) {
        json["$assetId"] = to_underlying(assetHandle->assetId());
    }
    return true;
}

bool up::reflex::_detail::encodeUuid(nlohmann::json& json, Schema const& schema, void const* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Uuid);

    auto const& uuid = *static_cast<UUID const*>(obj);
    if (!uuid.isValid()) {
        json = nullptr;
        return true;
    }

    char buf[UUID::strLength] = {
        0,
    };
    nanofmt::format_to(buf, "{}", uuid);

    json = buf;
    return true;
}

bool up::reflex::_detail::encodeValue(nlohmann::json& json, Schema const& schema, void const* obj) {
    switch (schema.primitive) {
        case SchemaPrimitive::Bool:
            json = *static_cast<bool const*>(obj);
            return true;
        case SchemaPrimitive::Int8:
            json = *static_cast<int8 const*>(obj);
            return true;
        case SchemaPrimitive::Int16:
            json = *static_cast<int16 const*>(obj);
            return true;
        case SchemaPrimitive::Int32:
            json = *static_cast<int32 const*>(obj);
            return true;
        case SchemaPrimitive::Int64:
            json = *static_cast<int64 const*>(obj);
            return true;
        case SchemaPrimitive::UInt8:
            json = *static_cast<uint8 const*>(obj);
            return true;
        case SchemaPrimitive::UInt16:
            json = *static_cast<uint16 const*>(obj);
            return true;
        case SchemaPrimitive::UInt32:
            json = *static_cast<uint32 const*>(obj);
            return true;
        case SchemaPrimitive::UInt64:
            json = *static_cast<uint64 const*>(obj);
            return true;
        case SchemaPrimitive::Vec3:
            json = nlohmann::json::array();
            json.push_back(static_cast<glm::vec3 const*>(obj)->x);
            json.push_back(static_cast<glm::vec3 const*>(obj)->y);
            json.push_back(static_cast<glm::vec3 const*>(obj)->z);
            return true;
        case SchemaPrimitive::Quat:
            return false;
        case SchemaPrimitive::Float:
            json = *static_cast<float const*>(obj);
            return true;
        case SchemaPrimitive::Double:
            json = *static_cast<double const*>(obj);
            return true;
        case SchemaPrimitive::Enum:
            json = enumToString(schema, readInt(*schema.elementType, obj));
            return true;
        case SchemaPrimitive::String:
            json = *static_cast<string const*>(obj);
            return true;
        case SchemaPrimitive::Pointer:
            if (schema.operations->pointerDeref != nullptr) {
                if (void const* pointee = schema.operations->pointerDeref(obj)) {
                    return encodeValue(json, *schema.elementType, pointee);
                }
                json = nullptr;
                return true;
            }
            return false;
        case SchemaPrimitive::Array:
            return encodeArray(json, schema, obj);
        case SchemaPrimitive::Object:
            return encodeObject(json, schema, obj);
        case SchemaPrimitive::AssetRef:
            return encodeAssetRef(json, schema, obj);
        case SchemaPrimitive::Uuid:
            return encodeUuid(json, schema, obj);
        default:
            return false;
    }
}

bool up::reflex::_detail::decodeObject(nlohmann::json const& json, Schema const& schema, void* obj) {
    if (!json.is_object()) {
        return false;
    }

    bool success = true;
    for (SchemaField const& field : schema.fields) {
        auto const* jsonName = queryAnnotation<schema::json>(field);
        char const* const fieldName = jsonName == nullptr ? field.name.c_str() : jsonName->name.c_str();
        if (json.contains(fieldName)) {
            success = decodeValue(json[fieldName], *field.schema, static_cast<char*>(obj) + field.offset) && success;
        }
    }
    return success;
}

bool up::reflex::_detail::decodeArray(nlohmann::json const& json, Schema const& schema, void* arr) {
    if (!json.is_array()) {
        return false;
    }

    if (schema.operations->arrayResize == nullptr) {
        return false;
    }
    if (schema.operations->arrayMutableElementAt == nullptr) {
        return false;
    }

    schema.operations->arrayResize(arr, json.size());

    bool success = true;
    for (size_t index = 0; index != json.size(); ++index) {
        void* el = schema.operations->arrayMutableElementAt(arr, index);
        success = decodeValue(json[index], *schema.elementType, el) && success;
    }

    return success;
}

bool up::reflex::_detail::decodeAssetRef(nlohmann::json const& json, Schema const& schema, void* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::AssetRef);

    auto* const assetHandle = static_cast<UntypedAssetHandle*>(obj);

    if (json.is_string()) {
        AssetKey key;
        key.uuid = UUID::fromString(json.get<string_view>());
        *assetHandle = UntypedAssetHandle(std::move(key));
    }
    else if (json.contains("uuid") && json["uuid"].is_string()) {
        AssetKey key;
        key.uuid = UUID::fromString(json["uuid"].get<string_view>());
        if (json.contains("logical") && json["logical"].is_string()) {
            key.logical = json["logical"].get<string>();
        }
        *assetHandle = UntypedAssetHandle(std::move(key));
    }
    else {
        *assetHandle = UntypedAssetHandle();
    }

    return true;
}

bool up::reflex::_detail::decodeUuid(nlohmann::json const& json, Schema const& schema, void* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Uuid);

    auto& uuid = *static_cast<UUID*>(obj);

    if (json.is_null()) {
        uuid = UUID{};
        return true;
    }

    if (json.is_string()) {
        uuid = UUID::fromString(json.get<string_view>());
        return true; // FIXME: error check result somehow?
    }

    return false;
}

template <typename T, typename U = T>
static bool decodeSimple(nlohmann::json const& json, void* obj) {
    *static_cast<T*>(obj) = json.get<U>();
    return true;
}

bool up::reflex::_detail::decodeValue(nlohmann::json const& json, Schema const& schema, void* obj) {
    switch (schema.primitive) {
        case SchemaPrimitive::Bool:
            return decodeSimple<bool>(json, obj);
        case SchemaPrimitive::Int8:
            return decodeSimple<int8>(json, obj);
        case SchemaPrimitive::Int16:
            return decodeSimple<int16>(json, obj);
        case SchemaPrimitive::Int32:
            return decodeSimple<int32>(json, obj);
        case SchemaPrimitive::Int64:
            return decodeSimple<int64>(json, obj);
        case SchemaPrimitive::UInt8:
            return decodeSimple<uint8>(json, obj);
        case SchemaPrimitive::UInt16:
            return decodeSimple<uint16>(json, obj);
        case SchemaPrimitive::UInt32:
            return decodeSimple<uint32>(json, obj);
        case SchemaPrimitive::UInt64:
            return decodeSimple<uint64>(json, obj);
        case SchemaPrimitive::Vec3:
            if (!json.is_array() || json.size() < 3) {
                return false;
            }
            if (!json[0].is_number() || !json[1].is_number() || !json[2].is_number()) {
                return false;
            }
            static_cast<glm::vec3*>(obj)->x = json[0].get<float>();
            static_cast<glm::vec3*>(obj)->y = json[1].get<float>();
            static_cast<glm::vec3*>(obj)->z = json[2].get<float>();
            return true;
        case SchemaPrimitive::Quat:
            return false;
        case SchemaPrimitive::Float:
            return decodeSimple<float>(json, obj);
        case SchemaPrimitive::Double:
            return decodeSimple<double>(json, obj);
        case SchemaPrimitive::Enum:
            writeInt(*schema.elementType, obj, enumToValue(schema, json.get<string_view>()));
            return true;
        case SchemaPrimitive::String:
            return decodeSimple<string>(json, obj);
        case SchemaPrimitive::Array:
            return decodeArray(json, schema, obj);
        case SchemaPrimitive::Object:
            return decodeObject(json, schema, obj);
        case SchemaPrimitive::Pointer:
            if (json.is_null() && schema.operations->pointerAssign != nullptr) {
                schema.operations->pointerAssign(obj, nullptr);
                return true;
            }
            else if (schema.operations->pointerMutableDeref != nullptr) {
                if (void* pointee = schema.operations->pointerMutableDeref(obj)) {
                    return decodeValue(json, *schema.elementType, pointee);
                }
            }

            // This needs some thought to handle polymorphic targets properly
            // else if (schema.operations->pointerInstantiate != nullptr) {
            //    if (void* pointee = schema.operations->pointerInstantiate(obj)) {
            //        return decodeValue(json, *schema.elementType, pointee);
            //    }
            //}
            return false;
        case SchemaPrimitive::AssetRef:
            return decodeAssetRef(json, schema, obj);
        case SchemaPrimitive::Uuid:
            return decodeUuid(json, schema, obj);
        default:
            return false;
    }
}

up::int64 up::reflex::_detail::readInt(Schema const& schema, void const* obj) {
    switch (schema.primitive) {
        case SchemaPrimitive::Int8:
            return static_cast<int64>(*reinterpret_cast<int8 const*>(obj));
        case SchemaPrimitive::UInt8:
            return static_cast<int64>(*reinterpret_cast<uint8 const*>(obj));
        case SchemaPrimitive::Int16:
            return static_cast<int64>(*reinterpret_cast<int16 const*>(obj));
        case SchemaPrimitive::UInt16:
            return static_cast<int64>(*reinterpret_cast<uint16 const*>(obj));
        case SchemaPrimitive::Int32:
            return static_cast<int64>(*reinterpret_cast<int32 const*>(obj));
        case SchemaPrimitive::UInt32:
            return static_cast<int64>(*reinterpret_cast<uint32 const*>(obj));
        case SchemaPrimitive::Int64:
            return *reinterpret_cast<int64 const*>(obj);
        case SchemaPrimitive::UInt64:
            return static_cast<int64>(*reinterpret_cast<uint64 const*>(obj));
        default:
            UP_UNREACHABLE("Incorrect primitive type");
            return 0;
    }
}

void up::reflex::_detail::writeInt(Schema const& schema, void* obj, int64 value) {
    switch (schema.primitive) {
        case SchemaPrimitive::Int8:
            *reinterpret_cast<int8*>(obj) = static_cast<int8>(value);
            break;
        case SchemaPrimitive::UInt8:
            *reinterpret_cast<uint8*>(obj) = static_cast<uint8>(value);
            break;
        case SchemaPrimitive::Int16:
            *reinterpret_cast<int16*>(obj) = static_cast<int16>(value);
            break;
        case SchemaPrimitive::UInt16:
            *reinterpret_cast<uint16*>(obj) = static_cast<uint16>(value);
            break;
        case SchemaPrimitive::Int32:
            *reinterpret_cast<int32*>(obj) = static_cast<int32>(value);
            break;
        case SchemaPrimitive::UInt32:
            *reinterpret_cast<uint32*>(obj) = static_cast<uint32>(value);
            break;
        case SchemaPrimitive::Int64:
            *reinterpret_cast<int64*>(obj) = value;
            break;
        case SchemaPrimitive::UInt64:
            *reinterpret_cast<uint64*>(obj) = static_cast<uint64>(value);
            break;
        default:
            UP_UNREACHABLE("Incorrect primitive type");
            break;
    }
}
