// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace schema {
    struct TypeBase;
    struct TypeAggregate;

    using Number = long long;
    using Value = std::variant<Number, std::string, std::nullptr_t, bool, TypeBase const*>;

    enum class TypeKind {
        Unknown,
        Simple,
        Struct,
        Attribute,
        Union,
        Alias,
        Enum,
        Pointer,
        Array,
        TypeParam,
        Specialized,
    };

    struct Entity {
        virtual ~Entity() = default;
    };

    struct Annotation : Entity {
        TypeAggregate const* type = nullptr;
        std::vector<Value> args;
    };

    using Annotations = std::vector<schema::Annotation const*>;

    struct Field : Entity {
        std::string name;
        TypeBase const* type = nullptr;
        Annotations annotations;
    };

    struct EnumItem : Entity {
        std::string name;
        Number value = 0;
    };

    struct TypeBase : Entity {
        TypeKind kind = TypeKind::Unknown;
        std::string name;
        std::string qualifiedName;
        std::string owningModule;
        Annotations annotations;
    };

    struct TypeIndirect : TypeBase {
        TypeBase const* ref = nullptr;
    };

    struct TypeArray : TypeIndirect {
        std::optional<Number> length;
    };

    struct TypeSpecialized : TypeIndirect {
        std::vector<TypeBase const*> typeArgs;
    };

    struct TypeAggregate : TypeBase {
        std::vector<Field const*> fields;
        std::vector<TypeBase const*> typeParams;
        TypeBase const* baseType = nullptr;
    };

    struct TypeEnum : TypeBase {
        std::vector<EnumItem const*> items;
    };

    struct Namespace : Entity {
        std::vector<TypeBase const*> types;
    };

    struct Module {
        std::string name;
        Namespace const* root = nullptr;
        std::vector<std::unique_ptr<Entity>> entities;
        std::vector<TypeBase const*> allTypes;
        std::vector<TypeBase const*> exportedTypes;
        std::vector<std::string> imports;
    };

    bool loadModule(std::istream& input, Module& mod);
    bool hasAnnotation(Annotations const& annotations, std::string_view name);
    std::optional<Value> getAnnotationArg(Annotations const& annotations, std::string_view name, size_t index = 0);

    template <typename ValueT>
    constexpr ValueT valueCast(Value const& value) {
        return std::visit(
            [](auto const& actual) -> ValueT {
                using ActualType = std::remove_cvref_t<decltype(actual)>;
                if constexpr (std::is_same_v<ValueT, std::string_view> && std::is_same_v<ActualType, std::string>) {
                    return actual;
                }
                if constexpr (std::is_same_v<ActualType, ValueT>) {
                    return actual;
                }
                return ValueT{};
            },
            value);
    }
} // namespace schema
