// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "schema.h"

#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>
#include <string_view>
#include <unordered_map>

template <typename EntT>
static EntT* createEntity(schema::Module& mod);

static schema::TypeKind kindFromString(std::string_view str) noexcept;
static schema::TypeBase const* findType(schema::Module const& mod, std::string_view qualifiedName) noexcept;

static schema::TypeBase* createType(schema::Module& mod, nlohmann::json const& json);

static void loadValue(schema::Value& value, schema::Module& mod, nlohmann::json const& json);
static void loadAnnotations(schema::Annotations& annotated, schema::Module& mod, nlohmann::json const& json);

static void loadType(schema::TypeBase& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeBase(schema::TypeBase& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeAggregate(schema::TypeAggregate& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeEnum(schema::TypeEnum& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeIndirect(schema::TypeIndirect& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeArray(schema::TypeArray& type, schema::Module& mod, nlohmann::json const& json);
static void loadTypeSpecialized(schema::TypeSpecialized& type, schema::Module& mod, nlohmann::json const& json);

bool schema::loadModule(std::istream& input, Module& mod) {
    auto const doc = nlohmann::json::parse(input, {}, /*allow_exceptions=*/false);
    if (doc.is_discarded()) {
        return false;
    }

    auto* const root = createEntity<Namespace>(mod);
    mod.root = root;

    std::unordered_map<std::string_view, schema::TypeBase*> typeMap;

    mod.name = doc["module"]["name"];

    // build list of types (so we can resolve all types in the second pass)
    for (auto const& type_json : doc["types"]) {
        schema::TypeBase* const type = createType(mod, type_json);
        if (type == nullptr) {
            return false;
        }
        typeMap.insert({type->qualifiedName, type});
        mod.allTypes.push_back(type);
    }

    // fill in type details
    for (auto const& type_json : doc["types"]) {
        auto const it = typeMap.find(type_json["qualified"].get<std::string_view>());
        assert(it != typeMap.end());
        schema::TypeBase* const type = it->second;
        loadType(*type, mod, type_json);
        root->types.push_back(type);

        if (type->owningModule == mod.name) {
            mod.exportedTypes.push_back(type);
        }
    }

    // fill in import details
    for (auto const& imp_json : doc["module"]["imports"]) {
        mod.imports.push_back(imp_json["name"]);
    }

    return true;
}

bool schema::hasAnnotation(Annotations const& annotations, std::string_view name) {
    for (auto const* anno : annotations) {
        if (anno->type->name == name) {
            return true;
        }
    }
    return false;
}

std::optional<schema::Value> schema::getAnnotationArg(
    Annotations const& annotations,
    std::string_view name,
    size_t index) {
    for (auto const* anno : annotations) {
        if (anno->type->name == name) {
            if (index < anno->args.size()) {
                return anno->args[index];
            }
            return {};
        }
    }
    return {};
}

template <typename EntT>
EntT* createEntity(schema::Module& mod) {
    return static_cast<EntT*>(mod.entities.emplace_back(std::make_unique<EntT>()).get());
}

schema::TypeKind kindFromString(std::string_view str) noexcept {
    using namespace std::literals;
    using namespace schema;

    if (str == "simple"sv) {
        return TypeKind::Simple;
    }
    if (str == "struct"sv) {
        return TypeKind::Struct;
    }
    if (str == "attribute"sv) {
        return TypeKind::Attribute;
    }
    if (str == "union"sv) {
        return TypeKind::Union;
    }
    if (str == "enum"sv) {
        return TypeKind::Enum;
    }
    if (str == "alias"sv) {
        return TypeKind::Alias;
    }
    if (str == "pointer"sv) {
        return TypeKind::Pointer;
    }
    if (str == "array"sv) {
        return TypeKind::Array;
    }
    if (str == "typeparam"sv) {
        return TypeKind::TypeParam;
    }
    if (str == "specialized"sv) {
        return TypeKind::Specialized;
    }

    return TypeKind::Unknown;
}

schema::TypeBase const* findType(schema::Module const& mod, std::string_view qualifiedName) noexcept {
    for (schema::TypeBase const* type : mod.allTypes) {
        if (type->name == qualifiedName) {
            return type;
        }
    }

    return nullptr;
}

schema::TypeBase* createType(schema::Module& mod, nlohmann::json const& json) {
    using namespace nlohmann;
    using namespace schema;

    auto const kind = kindFromString(json["kind"].get<std::string_view>());

    TypeBase* type = nullptr;

    if (kind == TypeKind::Struct || kind == TypeKind::Attribute || kind == TypeKind::Union) {
        type = createEntity<TypeAggregate>(mod);
    }
    else if (kind == TypeKind::Enum) {
        type = createEntity<TypeEnum>(mod);
    }
    else if (kind == TypeKind::Alias || kind == TypeKind::Pointer) {
        type = createEntity<TypeIndirect>(mod);
    }
    else if (kind == TypeKind::Array) {
        type = createEntity<TypeArray>(mod);
    }
    else if (kind == TypeKind::Specialized) {
        type = createEntity<TypeSpecialized>(mod);
    }
    else {
        type = createEntity<TypeBase>(mod);
    }

    type->kind = kind;
    type->name = json["name"];
    type->qualifiedName = json["qualified"];
    return type;
}

void loadValue(schema::Value& value, schema::Module& mod, nlohmann::json const& json) {
    using namespace schema;

    if (json.is_null()) {
        value = nullptr;
    }
    else if (json.is_string()) {
        value = std::string{json};
    }
    else if (json.is_number()) {
        value = Number{json};
    }
    else if (json.is_boolean()) {
        value = bool{json};
    }
}

void loadAnnotations(schema::Annotations& annotated, schema::Module& mod, nlohmann::json const& json) {
    using namespace schema;

    if (!json.contains("annotations")) {
        return;
    }

    for (auto const& anno_json : json["annotations"]) {
        auto* const anno = createEntity<Annotation>(mod);
        anno->type = static_cast<TypeAggregate const*>(findType(mod, anno_json["type"].get<std::string_view>()));
        for (auto const& arg_json : anno_json["args"]) {
            loadValue(anno->args.emplace_back(), mod, arg_json);
        }
        annotated.push_back(anno);
    }
}

void loadType(schema::TypeBase& type, schema::Module& mod, nlohmann::json const& json) {
    using namespace schema;

    if (type.kind == TypeKind::Struct || type.kind == TypeKind::Attribute || type.kind == TypeKind::Union) {
        return loadTypeAggregate(static_cast<TypeAggregate&>(type), mod, json);
    }
    if (type.kind == TypeKind::Enum) {
        return loadTypeEnum(static_cast<TypeEnum&>(type), mod, json);
    }
    if (type.kind == TypeKind::Alias || type.kind == TypeKind::Pointer) {
        return loadTypeIndirect(static_cast<TypeIndirect&>(type), mod, json);
    }
    if (type.kind == TypeKind::Array) {
        return loadTypeArray(static_cast<TypeArray&>(type), mod, json);
    }
    if (type.kind == TypeKind::Specialized) {
        return loadTypeSpecialized(static_cast<TypeSpecialized&>(type), mod, json);
    }

    return loadTypeBase(type, mod, json);
}

void loadTypeBase(schema::TypeBase& type, schema::Module& mod, nlohmann::json const& json) {
    loadAnnotations(type.annotations, mod, json);

    type.owningModule = json["module"];
}

void loadTypeAggregate(schema::TypeAggregate& type, schema::Module& mod, nlohmann::json const& json) {
    using namespace schema;

    loadTypeBase(type, mod, json);

    if (json.contains("base")) {
        type.baseType = findType(mod, json["base"].get<std::string_view>());
    }

    for (auto const& field_json : json["fields"]) {
        auto* const field = createEntity<Field>(mod);
        type.fields.push_back(field);

        field->name = field_json["name"];
        field->type = findType(mod, field_json["type"].get<std::string_view>());

        loadAnnotations(field->annotations, mod, field_json);
    }

    if (json.contains("typeParams")) {
        for (auto const& type_json : json["typeParams"]) {
            type.typeParams.push_back(findType(mod, type_json.get<std::string_view>()));
        }
    }
}

void loadTypeEnum(schema::TypeEnum& type, schema::Module& mod, nlohmann::json const& json) {
    using namespace schema;

    loadTypeBase(type, mod, json);

    for (auto const& item_json : json["items"]) {
        auto* const item = createEntity<EnumItem>(mod);
        type.items.push_back(item);

        item->name = item_json["name"];
        item->value = item_json["value"];
    }
}

void loadTypeIndirect(schema::TypeIndirect& type, schema::Module& mod, nlohmann::json const& json) {
    loadTypeBase(type, mod, json);

    type.ref = findType(mod, json["refType"].get<std::string_view>());
}

void loadTypeArray(schema::TypeArray& type, schema::Module& mod, nlohmann::json const& json) {
    loadTypeIndirect(type, mod, json);

    if (json.contains("length")) {
        type.length = json["length"];
    }
}

void loadTypeSpecialized(schema::TypeSpecialized& type, schema::Module& mod, nlohmann::json const& json) {
    loadTypeIndirect(type, mod, json);

    for (auto const& type_json : json["typeArgs"]) {
        type.typeArgs.push_back(findType(mod, type_json.get<std::string_view>()));
    }
}
