// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"
#include "type.h"

#include "potato/spud/box.h"
#include "potato/spud/concepts.h"
#include "potato/spud/hash.h"
#include "potato/spud/key.h"
#include "potato/spud/rc.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/traits.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <glm/fwd.hpp>

namespace up {
    class string;
    class UUID;
} // namespace up

namespace up::reflex {
    struct TypeInfo;
    struct SchemaType;
    struct Schema;

    struct SchemaId : Key<SchemaId> {
        using Key<SchemaId>::Key;
    };

    struct SchemaAttribute { };

    template <typename T>
    struct TypeHolder;

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
        Enum,
        Vec3,
        Float,
        Double,
        Pointer,
        String,
        Array,
        Object,
        AssetRef,
        Uuid,
    };

    struct SchemaAnnotation {
        TypeInfo const* type = nullptr;
        SchemaAttribute const* attr = nullptr;
    };

    struct SchemaOperations {
        using ArrayGetSize = size_t (*)(void const* arr);
        using ArrayElementAt = void const* (*)(void const* arr, size_t index);
        using ArrayMutableElementAt = void* (*)(void* arr, size_t index);
        using ArrayMoveTo = void (*)(void* arr, size_t to, size_t from);
        using ArrayEraseAt = void (*)(void* arr, size_t index);
        using ArrayResize = void (*)(void* arr, size_t size);

        using PointerDeref = void const* (*)(void const* ptr);
        using PointerMutableDeref = void* (*)(void* ptr);
        using PointerAssign = void (*)(void* ptr, void* object);

        ArrayGetSize arrayGetSize = nullptr;
        ArrayElementAt arrayElementAt = nullptr;
        ArrayMutableElementAt arrayMutableElementAt = nullptr;
        ArrayMoveTo arrayMoveTo = nullptr;
        ArrayEraseAt arrayEraseAt = nullptr;
        ArrayResize arrayResize = nullptr;

        PointerDeref pointerDeref = nullptr;
        PointerMutableDeref pointerMutableDeref = nullptr;
        PointerAssign pointerAssign = nullptr;
    };

    struct SchemaField {
        zstring_view name;
        Schema const* schema = nullptr;
        int offset = 0;
        view<SchemaAnnotation> annotations{};
    };

    struct SchemaEnumValue {
        zstring_view name;
        int64 value = 0;
    };

    struct Schema {
        SchemaId id;
        zstring_view name;
        SchemaPrimitive primitive = SchemaPrimitive::Null;
        Schema const* baseSchema = nullptr;
        Schema const* elementType = nullptr;
        SchemaOperations const* operations = nullptr;
        view<SchemaField> fields{};
        view<SchemaEnumValue> enumValues{};
        view<SchemaAnnotation> annotations{};

        template <typename AttributeT>
        AttributeT const* queryAnnotation() const noexcept requires(std::is_base_of_v<SchemaAttribute, AttributeT>) {
            TypeInfo const& type = TypeHolder<AttributeT>::get();
            for (SchemaAnnotation const& anno : annotations) {
                if (anno.type->hash == type.hash) {
                    return static_cast<AttributeT const*>(anno.attr);
                }
            }
            return nullptr;
        }
    };

    template <typename T>
    concept IsSchemaAnnotation = std::is_base_of_v<SchemaAttribute, T>;

    template <typename T>
    struct SchemaHolder;

    template <typename T>
    concept schema_glm_type = std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::quat>;
    template <typename T>
    concept schema_primitive = std::is_scalar_v<T> || std::is_same_v<T, string> || schema_glm_type<T> || is_vector_v<T>;
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
    Schema const& getSchema() noexcept {
        using Type = std::remove_cv_t<std::decay_t<T>>;
        if constexpr (std::is_same_v<Type, bool>) {
            static constexpr Schema schema{.id = SchemaId(1), .name = "bool"_zsv, .primitive = SchemaPrimitive::Bool};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int8> || (std::is_same_v<Type, char> && std::is_signed_v<char>)) {
            static constexpr Schema schema{.id = SchemaId(2), .name = "int8"_zsv, .primitive = SchemaPrimitive::Int8};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int16>) {
            static constexpr Schema schema{.id = SchemaId(3), .name = "int16"_zsv, .primitive = SchemaPrimitive::Int16};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int32>) {
            static constexpr Schema schema{.id = SchemaId(4), .name = "int32"_zsv, .primitive = SchemaPrimitive::Int32};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, int64>) {
            static constexpr Schema schema{.id = SchemaId(5), .name = "int64"_zsv, .primitive = SchemaPrimitive::Int64};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, uint8> || (std::is_same_v<Type, char> && std::is_unsigned_v<char>)) {
            static constexpr Schema schema{.id = SchemaId(6), .name = "uint8"_zsv, .primitive = SchemaPrimitive::UInt8};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, uint16>) {
            static constexpr Schema schema{
                .id = SchemaId(7),
                .name = "uint16"_zsv,
                .primitive = SchemaPrimitive::UInt16};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, uint32>) {
            static constexpr Schema schema{
                .id = SchemaId(8),
                .name = "uint32"_zsv,
                .primitive = SchemaPrimitive::UInt32};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, uint64>) {
            static constexpr Schema schema{
                .id = SchemaId(9),
                .name = "uint64"_zsv,
                .primitive = SchemaPrimitive::UInt64};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, float>) {
            static constexpr Schema schema{
                .id = SchemaId{10},
                .name = "float"_zsv,
                .primitive = SchemaPrimitive::Float};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, double>) {
            static constexpr Schema schema{
                .id = SchemaId{11},
                .name = "double"_zsv,
                .primitive = SchemaPrimitive::Double};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, glm::vec3>) {
            static constexpr Schema schema{.id = SchemaId{12}, .name = "vec3"_zsv, .primitive = SchemaPrimitive::Vec3};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, string>) {
            static constexpr Schema schema{
                .id = SchemaId{13},
                .name = "string"_zsv,
                .primitive = SchemaPrimitive::String};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, std::nullptr_t>) {
            static constexpr Schema schema{
                .id = SchemaId{14},
                .name = "nullptr"_zsv,
                .primitive = SchemaPrimitive::Pointer};
            return schema;
        }
        else if constexpr (std::is_same_v<Type, up::UUID>) {
            static constexpr Schema schema{.id = SchemaId{15}, .name = "uuid"_zsv, .primitive = SchemaPrimitive::Uuid};
            return schema;
        }
        else if constexpr (is_vector_v<Type>) {
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static SchemaOperations const operations = {
                .arrayGetSize =
                    [](void const* array) noexcept {
                        auto const& arr = *static_cast<Type const*>(array);
                        return arr.size();
                    },
                .arrayElementAt = [](void const* array, size_t index) noexcept -> void const* {
                    auto const& arr = *static_cast<Type const*>(array);
                    return arr.data() + index;
                },
                .arrayMutableElementAt = [](void* array, size_t index) noexcept -> void* {
                    auto& arr = *static_cast<Type*>(array);
                    return arr.data() + index;
                },
                .arrayMoveTo =
                    [](void* array, size_t to, size_t from) noexcept {
                        auto& arr = *static_cast<Type*>(array);
                        int const step = to > from ? +1 : -1;
                        while (from != to) {
                            using std::swap;
                            size_t const next = from + step;
                            swap(arr[from], arr[next]);
                            from = next;
                        }
                    },
                .arrayEraseAt =
                    [](void* array, size_t index) {
                        auto& arr = *static_cast<Type*>(array);
                        arr.erase(arr.begin() + index);
                    },
                .arrayResize =
                    [](void* array, size_t size) {
                        auto& arr = *static_cast<Type*>(array);
                        arr.resize(size);
                    }};

            static Schema const schema{
                .id = SchemaId(hash_combine(elementSchema.id.value(), hash_value("vector"_sv))),
                .name = "vector"_zsv,
                .primitive = SchemaPrimitive::Array,
                .elementType = &elementSchema,
                .operations = &operations};
            return schema;
        }
        else if constexpr (is_box_v<Type>) {
            using ValueType = typename Type::value_type;
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static SchemaOperations const operations = {
                .pointerDeref = [](void const* ptr) -> void const* { return static_cast<Type const*>(ptr)->get(); },
                .pointerMutableDeref = [](void* ptr) -> void* { return static_cast<Type const*>(ptr)->get(); },
                .pointerAssign = [](void* ptr,
                                    void* object) { static_cast<Type*>(ptr)->reset(static_cast<ValueType*>(object)); },
            };

            static Schema const schema{
                .id = SchemaId(hash_combine(elementSchema.id.value(), hash_value("box"_sv))),
                .name = "box"_zsv,
                .primitive = SchemaPrimitive::Pointer,
                .elementType = &elementSchema,
                .operations = &operations};
            return schema;
        }
        else if constexpr (is_rc_v<Type>) {
            using ValueType = typename Type::value_type;
            static Schema const& elementSchema = getSchema<typename Type::value_type>();
            static SchemaOperations const operations = {
                .pointerDeref = [](void const* ptr) -> void const* { return static_cast<Type const*>(ptr)->get(); },
                .pointerMutableDeref = [](void* ptr) -> void* { return static_cast<Type const*>(ptr)->get(); },
                .pointerAssign = [](void* ptr,
                                    void* object) { static_cast<Type*>(ptr)->reset(static_cast<ValueType*>(object)); },
            };

            static Schema const schema{
                .id = SchemaId(hash_combine(elementSchema.id.value(), hash_value("rc"_sv))),
                .name = "rc"_zsv,
                .primitive = SchemaPrimitive::Pointer,
                .elementType = &elementSchema,
                .operations = &operations};
            return schema;
        }
        else {
            return SchemaHolder<T>::get();
        }
    }

    template <IsSchemaAnnotation AttributeT>
    constexpr auto queryAnnotation(view<SchemaAnnotation> annotations) noexcept -> AttributeT const* {
        TypeInfo const& type = TypeHolder<AttributeT>::get();
        for (SchemaAnnotation const& anno : annotations) {
            if (anno.type->hash == type.hash) {
                return static_cast<AttributeT const*>(anno.attr);
            }
        }
        return nullptr;
    }

    template <IsSchemaAnnotation AttributeT>
    constexpr auto queryAnnotation(Schema const& schema) noexcept -> AttributeT const* {
        return queryAnnotation<AttributeT>(schema.annotations);
    }

    template <IsSchemaAnnotation AttributeT>
    constexpr auto queryAnnotation(SchemaField const& field) noexcept -> AttributeT const* {
        return queryAnnotation<AttributeT>(field.annotations);
    }

    constexpr auto enumToString(Schema const& schema, int64 value) noexcept -> zstring_view {
        for (SchemaEnumValue const& enVal : schema.enumValues) {
            if (enVal.value == value) {
                return enVal.name;
            }
        }
        return {};
    }

    template <enumeration EnumT>
    constexpr auto enumToString(EnumT en) noexcept -> zstring_view {
        return enumToString(getSchema<EnumT>(), static_cast<int64>(en));
    }

    constexpr auto enumToValue(Schema const& schema, string_view name, int64 otherwise = 0) noexcept -> int64 {
        for (SchemaEnumValue const& enVal : schema.enumValues) {
            if (enVal.name == name) {
                return enVal.value;
            }
        }
        return otherwise;
    }

    template <enumeration EnumT>
    constexpr auto enumToValue(string_view name, EnumT otherwise = {}) noexcept -> EnumT {
        return static_cast<EnumT>(enumToValue(getSchema<EnumT>(), name, static_cast<int64>(otherwise)));
    }
} // namespace up::reflex
