// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/shell/scene_doc.h"

#include "potato/game/common.h"
#include "potato/game/space.h"
#include "potato/game/world.h"
#include "potato/reflex/serialize.h"
#include "potato/render/mesh.h"
#include "potato/schema/scene_schema.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/json.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

auto up::SceneDatabase::createByName(string_view name) -> box<SceneComponent> {
    for (EditComponent const& component : components()) {
        reflex::TypeInfo const& typeInfo = component.typeInfo();
        if (typeInfo.name == name) {
            return createByType(component);
        }
    }

    return nullptr;
}

auto up::SceneDatabase::createByType(EditComponent const& component) -> box<SceneComponent> {
    reflex::TypeInfo const& typeInfo = component.typeInfo();

    auto sceneComp = new_box<SceneComponent>();
    sceneComp->name = string{typeInfo.name};
    sceneComp->info = &component;
    sceneComp->data.reset(operator new(typeInfo.size));
    typeInfo.ops.defaultConstructor(sceneComp->data.get());
    return sceneComp;
}

int up::SceneDocument::indexOf(SceneEntityId entityId) const noexcept {
    for (int index : indices()) {
        if (_entities[index].sceneId == entityId) {
            return index;
        }
    }
    return -1;
}

up::SceneEntityId up::SceneDocument::createEntity(string name, SceneEntityId parentId) {
    SceneEntityId const id = _allocateEntityId();

    _entities.push_back({.name = std::move(name), .sceneId = id});
    if (parentId != SceneEntityId::None) {
        parentTo(id, parentId);
    }
    return id;
}

void up::SceneDocument::deleteEntity(SceneEntityId targetId) {
    auto const index = indexOf(targetId);
    if (index == -1) {
        return;
    }

    // recursively delete childen - we have to buffer this since deleting a child
    // would mutate the list we're walking
    vector<SceneEntityId> deleted{targetId};
    _deleteEntityAt(index, deleted);

    for (SceneEntityId const entityId : deleted) {
        _entities.erase(_entities.begin() + indexOf(entityId));
    }
}

void up::SceneDocument::_deleteEntityAt(int index, vector<SceneEntityId>& out_deleted) {
    // reparent to ensure we're unlinked from a parent's chain
    parentTo(_entities[index].sceneId, SceneEntityId::None);

    while (_entities[index].firstChild != -1) {
        out_deleted.push_back(_entities[_entities[index].firstChild].sceneId);
        _deleteEntityAt(_entities[index].firstChild, out_deleted);
    }
}

void up::SceneDocument::parentTo(SceneEntityId childId, SceneEntityId parentId) {
    int const childIndex = indexOf(childId);
    if (childIndex == -1) {
        return;
    }
    SceneEntity& childEnt = _entities[childIndex];

    int const parentIndex = indexOf(parentId);
    if (parentIndex == childEnt.parent) {
        return;
    }

    // remove from old parent
    if (childEnt.parent != -1) {
        SceneEntity& oldParent = _entities[childEnt.parent];
        if (oldParent.firstChild == childIndex) {
            oldParent.firstChild = childEnt.nextSibling;
        }
        else {
            for (int index = oldParent.firstChild; index != -1; index = _entities[index].nextSibling) {
                if (_entities[index].nextSibling == childIndex) {
                    _entities[index].nextSibling = childEnt.nextSibling;
                    break;
                }
            }
        }

        childEnt.parent = -1;
        childEnt.nextSibling = -1;
    }

    // add to new parent
    if (parentIndex != -1) {
        SceneEntity& newParent = _entities[parentIndex];
        if (newParent.firstChild == -1) {
            newParent.firstChild = childIndex;
        }
        else {
            for (int index = newParent.firstChild;; index = _entities[index].nextSibling) {
                if (_entities[index].nextSibling == -1) {
                    _entities[index].nextSibling = childIndex;
                    break;
                }
            }
        }

        childEnt.parent = parentIndex;
        childEnt.nextSibling = -1;
    }
}

auto up::SceneDocument::addNewComponent(SceneEntityId entityId, EditComponent const& component) -> SceneComponent* {
    int const index = indexOf(entityId);
    if (index == -1) {
        return nullptr;
    }
    SceneEntity& entity = _entities[index];

    reflex::TypeInfo const& typeInfo = component.typeInfo();

    auto& sceneComp = entity.components.push_back(_database.createByType(component));
    sceneComp->name = string{typeInfo.name};
    sceneComp->parent = entityId;
    sceneComp->info = &component;
    sceneComp->data.reset(operator new(typeInfo.size));
    typeInfo.ops.defaultConstructor(sceneComp->data.get());
    return sceneComp.get();
}

void up::SceneDocument::createTestObjects(
    Mesh::Handle const& cube,
    Material::Handle const& mat,
    SoundResource::Handle const& ding) {
    auto pi = glm::pi<float>();

    constexpr int numObjects = 100;

    auto addComponentData = [this]<typename ComponentT>(SceneEntityId entityId, ComponentT&& component) {
        auto comp = _database.createByName(reflex::getTypeInfo<ComponentT>().name);
        *static_cast<ComponentT*>(comp->data.get()) = std::forward<ComponentT>(component);
    };

    auto const rootId = createEntity("Root");

    auto const centerId = createEntity("Center", rootId);
    addComponentData(
        centerId,
        scene::components::Transform{.position = {0, 5, 0}, .rotation = glm::identity<glm::quat>()});
    addComponentData(centerId, scene::components::Mesh{.mesh = cube, .material = mat});
    addComponentData(centerId, scene::components::Ding{.period = 2, .sound = {ding}});

    auto const ringId = createEntity("Ring", rootId);
    for (size_t i = 0; i <= numObjects; ++i) {
        float p = static_cast<float>(i) / static_cast<float>(numObjects);
        float r = p * 2.f * pi;
        auto const id = createEntity("Orbit", ringId);
        addComponentData(
            id,
            scene::components::Transform{
                .position =
                    {(20 + glm::cos(r) * 10.f) * glm::sin(r),
                     1 + glm::sin(r * 10.f) * 5.f,
                     (20 + glm::sin(r) * 10.f) * glm::cos(r)},
                .rotation = glm::identity<glm::quat>()});
        addComponentData(id, scene::components::Mesh{.mesh = cube, .material = mat});
        addComponentData(id, scene::components::Wave{.offset = r});
        addComponentData(id, scene::components::Spin{.radians = glm::sin(r) * 2.f - 1.f});
    }
}

void up::SceneDocument::syncPreview(Space& space) {
    for (auto& entity : _entities) {
        if (entity.previewId == EntityId::None) {
            entity.previewId = space.world().createEntity();
        }

        for (auto& component : entity.components) {
            switch (component->state) {
                case SceneComponent::State::Idle:
                    break;
                case SceneComponent::State::New:
                    component->info->syncAdd(space, entity.previewId, *component);
                    component->state = SceneComponent::State::Idle;
                    break;
                case SceneComponent::State::Pending:
                    component->info->syncUpdate(space, entity.previewId, *component);
                    component->state = SceneComponent::State::Idle;
                    break;
                case SceneComponent::State::Removed:
                    component->info->syncRemove(space, entity.previewId, *component);
                    component->state = SceneComponent::State::Idle;
                    break;
            }
        }
    }
}

void up::SceneDocument::syncGame(Space& space) const {
    for (auto& entity : _entities) {
        EntityId entityId = space.world().createEntity();

        for (auto& component : entity.components) {
            component->info->syncGame(space, entityId, *component);
        }
    }
}

void up::SceneDocument::toJson(nlohmann::json& doc) const {
    doc = nlohmann::json::object();
    doc["$type"] = "potato.document.scene";
    _toJson(doc["objects"], 0);
}

void up::SceneDocument::_toJson(nlohmann::json& el, int index) const {
    SceneEntity const& ent = _entities[index];
    el = nlohmann::json::object();
    el["name"] = ent.name;

    nlohmann::json& components = el["components"] = nlohmann::json::array();
    for (auto& component : ent.components) {
        nlohmann::json compEl = nlohmann::json::object();
        reflex::encodeToJsonRaw(compEl, *component->info->typeInfo().schema, component->data.get());
        components.push_back(std::move(compEl));
    }

    if (ent.firstChild != -1) {
        nlohmann::json& children = el["children"] = nlohmann::json::array();
        for (int childIndex = ent.firstChild; childIndex != -1; childIndex = _entities[childIndex].nextSibling) {
            nlohmann::json childEl{};
            _toJson(childEl, childIndex);
            children.push_back(std::move(childEl));
        }
    }
}

void up::SceneDocument::fromJson(nlohmann::json const& doc, AssetLoader& assetLoader) {
    _entities.clear();

    if (doc.contains("objects") && doc["objects"].is_object()) {
        int const index = static_cast<int>(_entities.size());
        _entities.emplace_back();
        _fromJson(doc["objects"], index, assetLoader);
    }
}

void up::SceneDocument::_fromJson(nlohmann::json const& el, int index, AssetLoader& assetLoader) {
    if (el.contains("name") && el["name"].is_string()) {
        _entities[index].name = el["name"].get<string>();
    }

    if (el.contains("id") && el["id"].is_number_integer()) {
        _entities[index].sceneId = _consumeEntityId(SceneEntityId{el["id"].get<uint64>()});
    }
    else {
        _entities[index].sceneId = _allocateEntityId();
    }

    if (el.contains("components") && el["components"].is_array()) {
        for (nlohmann::json const& compEl : el["components"]) {
            if (!compEl.contains("$schema") || !compEl["$schema"].is_string()) {
                continue;
            }

            auto const name = compEl["$schema"].get<string_view>();
            box<SceneComponent> component = _database.createByName(name);
            if (component == nullptr) {
                continue;
            }

            if (compEl.contains("name") && compEl["name"].is_string()) {
                auto name = compEl["name"].get<string>();
                if (!name.empty()) {
                    component->name = std::move(name);
                }
            }

            component->parent = _entities[index].sceneId;

            reflex::decodeFromJsonRaw(compEl, *component->info->typeInfo().schema, component->data.get());

            for (reflex::SchemaField const& field : component->info->typeInfo().schema->fields) {
                if (field.schema->primitive == reflex::SchemaPrimitive::AssetRef) {
                    auto* const assetHandle = reinterpret_cast<UntypedAssetHandle*>(
                        reinterpret_cast<char*>(component->data.get()) + field.offset);
                    *assetHandle = assetLoader.loadAssetSync(assetHandle->assetId());
                }
            }

            _entities[index].components.push_back(std::move(component));
        }
    }

    if (el.contains("children") && el["children"].is_array()) {
        int prevSibling = -1;
        for (nlohmann::json const& childEl : el["children"]) {
            int const childIndex = static_cast<int>(_entities.size());
            _entities.push_back({.parent = index});
            if (prevSibling == -1) {
                _entities[index].firstChild = childIndex;
            }
            else {
                _entities[prevSibling].nextSibling = childIndex;
            }
            prevSibling = childIndex;
            _fromJson(childEl, childIndex, assetLoader);
        }
    }
}

auto up::SceneDocument::_allocateEntityId() -> SceneEntityId {
    SceneEntityId const id = _nextEntityId;
    _nextEntityId = SceneEntityId{to_underlying(_nextEntityId) + 1};
    return id;
}

auto up::SceneDocument::_consumeEntityId(SceneEntityId entityId) -> SceneEntityId {
    if (to_underlying(entityId) >= to_underlying(_nextEntityId)) {
        _nextEntityId = SceneEntityId{to_underlying(entityId) + 1};
    }
    return entityId;
}
