// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <glm/fwd.hpp>

namespace up {
    class string;
}

namespace up::reflex {
    class TypeInfo;
    class Attribute;
    struct SchemaType;
    struct Schema;

    enum class SchemaPrimitive {
        Null,
        Bool,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Vec3,
        Mat4x4,
        Quat,
        Float,
        Double,
        Pointer,
        String,
        Array,
        Object
    };

    struct SchemaAnnotation {
        TypeInfo const* type = nullptr;
        Attribute const* attr = nullptr;
    };

    struct SchemaField {
        zstring_view name;
        Schema const* schema = nullptr;
        int offset = 0;
        view<SchemaAnnotation> annotations;

        template <typename AttributeT>
        AttributeT const* queryAnnotation() const noexcept {
            TypeInfo const& type = getTypeInfo<AttributeT>();
            for (SchemaAnnotation const& anno : annotations) {
                if (anno.type == &type) {
                    return static_cast<AttributeT const*>(anno.attr);
                }
            }
            return nullptr;
        }
    };

    struct Schema {
        zstring_view name;
        SchemaPrimitive primitive = SchemaPrimitive::Null;
        Schema const* elementType = nullptr;
        view<SchemaField> fields;
        view<SchemaAnnotation> annotations;

        template <typename AttributeT>
        AttributeT const* queryAnnotation() const noexcept {
            TypeInfo const& type = getTypeInfo<AttributeT>();
            for (SchemaAnnotation const& anno : annotations) {
                if (anno.type == &type) {
                    return static_cast<AttributeT const*>(anno.attr);
                }
            }
            return nullptr;
        }
    };

    template <typename T>
    struct SchemaHolder;

    template <typename T>
    concept schema_glm_type =
        std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::mat4x4> || std::is_same_v<T, glm::quat>;
    template <typename T>
    concept schema_primitive = std::is_scalar_v<T> || std::is_same_v<T, string> || schema_glm_type<T>;
    template <typename T>
    concept has_schema = requires(T t) {
        {SchemaHolder<T>::get()};
    }
    || requires(T t) {
        typename T::value_type;
        {SchemaHolder<typename T::value_type>::get()};
    }
    || schema_primitive<T>;

    template <has_schema T>
    constexpr Schema const& getSchema() noexcept {
        using Type = std::remove_cv_t<std::decay_t<T>>;
        if constexpr (std::is_same_v<Type, bool>) {
            static constexpr Schema schema{.name = "bool"_zsv, .primitive = SchemaPrimitive::Bool};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int8> || (std::is_same_v<Type, char> && std::is_signed_v<char>)) {
            static constexpr Schema schema{.name = "int8"_zsv, .primitive = SchemaPrimitive::Int8};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int16>) {
            static constexpr Schema schema{.name = "int16"_zsv, .primitive = SchemaPrimitive::Int16};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int32>) {
            static constexpr Schema schema{.name = "int32"_zsv, .primitive = SchemaPrimitive::Int32};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int64>) {
            static constexpr Schema schema{.name = "int64"_zsv, .primitive = SchemaPrimitive::Int64};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int8> || (std::is_same_v<Type, char> && std::is_unsigned_v<char>)) {
            static constexpr Schema schema{.name = "uint8"_zsv, .primitive = SchemaPrimitive::UInt8};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int16>) {
            static constexpr Schema schema{.name = "uint16"_zsv, .primitive = SchemaPrimitive::UInt16};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int32>) {
            static constexpr Schema schema{.name = "uint32"_zsv, .primitive = SchemaPrimitive::UInt32};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int64>) {
            static constexpr Schema schema{.name = "uint64"_zsv, .primitive = SchemaPrimitive::UInt64};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, float>) {
            static constexpr Schema schema{.name = "float"_zsv, .primitive = SchemaPrimitive::Float};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, double>) {
            static constexpr Schema schema{.name = "double"_zsv, .primitive = SchemaPrimitive::Double};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, glm::vec3>) {
            static constexpr Schema schema{.name = "vec3"_zsv, .primitive = SchemaPrimitive::Vec3};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, glm::mat4x4>) {
            static constexpr Schema schema{.name = "mat4x4"_zsv, .primitive = SchemaPrimitive::Mat4x4};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, glm::quat>) {
            static constexpr Schema schema{.name = "quat"_zsv, .primitive = SchemaPrimitive::Quat};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, string>) {
            static constexpr Schema schema{.name = "string"_zsv, .primitive = SchemaPrimitive::String};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, std::nullptr_t>) {
            static constexpr Schema schema{.name = "nullptr"_zsv, .primitive = SchemaPrimitive::Pointer};
            return schema;
        }
        else if constexpr (is_vector_v<Type>) {
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static Schema const schema{
                .name = "vector"_zsv,
                .primitive = SchemaPrimitive::Array,
                .elementType = &elementSchema};
            return schema;
        }
        else if constexpr (is_box_v<Type>) {
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static Schema const schema{
                .name = "box"_zsv,
                .primitive = SchemaPrimitive::Pointer,
                .elementType = &elementSchema};
            return schema;
        }
        else if constexpr (is_rc_v<Type>) {
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static Schema const schema{
                .name = "box"_zsv,
                .primitive = SchemaPrimitive::Pointer,
                .elementType = &elementSchema};
            return schema;
        }
        else {
            return SchemaHolder<T>::get();
        }
    }
} // namespace up::reflex
