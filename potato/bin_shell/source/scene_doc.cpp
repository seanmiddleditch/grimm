// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_doc.h"
#include "components_schema.h"

#include "potato/ecs/world.h"
#include "potato/reflex/serialize.h"
#include "potato/render/mesh.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/json.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

int up::SceneDocument::indexOf(EntityId entityId) const noexcept {
    for (int index : indices()) {
        if (_entities[index].id == entityId) {
            return index;
        }
    }
    return -1;
}

up::EntityId up::SceneDocument::createEntity(string name, EntityId parentId) {
    EntityId const id = _scene->world().createEntity();
    _entities.push_back({.name = std::move(name), .id = id});
    if (parentId != EntityId::None) {
        parentTo(id, parentId);
    }
    return id;
}

void up::SceneDocument::deleteEntity(EntityId targetId) {
    auto const index = indexOf(targetId);
    if (index == -1) {
        return;
    }

    // recursively delete childen - we have to buffer this since deleting a child
    // would mutate the list we're walking
    vector<EntityId> deleted{targetId};
    _deleteEntityAt(index, deleted);

    for (EntityId const childId : deleted) {
        _entities.erase(_entities.begin() + indexOf(childId));
        _scene->world().deleteEntity(childId);
    }
}

void up::SceneDocument::_deleteEntityAt(int index, vector<EntityId>& out_deleted) {
    // reparent to ensure we're unlinked from a parent's chain
    parentTo(_entities[index].id, EntityId::None);

    while (_entities[index].firstChild != -1) {
        out_deleted.push_back(_entities[_entities[index].firstChild].id);
        _deleteEntityAt(_entities[index].firstChild, out_deleted);
    }
}

void up::SceneDocument::parentTo(EntityId childId, EntityId parentId) {
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

void up::SceneDocument::createTestObjects(
    Mesh::Handle const& cube,
    Material::Handle const& mat,
    SoundResource::Handle const& ding) {
    auto pi = glm::pi<float>();

    constexpr int numObjects = 100;

    auto const rootId = createEntity("Root");

    auto const centerId = createEntity("Center", rootId);
    _scene->world().addComponent(
        centerId,
        components::Transform{.position = {0, 5, 0}, .rotation = glm::identity<glm::quat>()});
    _scene->world().addComponent(centerId, components::Mesh{cube, mat});
    _scene->world().addComponent(centerId, components::Ding{2, 0, {ding}});

    auto const ringId = createEntity("Ring", rootId);
    for (size_t i = 0; i <= numObjects; ++i) {
        float p = static_cast<float>(i) / static_cast<float>(numObjects);
        float r = p * 2.f * pi;
        auto const id = createEntity("Orbit", ringId);
        _scene->world().addComponent(
            id,
            components::Transform{
                .position =
                    {(20 + glm::cos(r) * 10.f) * glm::sin(r),
                     1 + glm::sin(r * 10.f) * 5.f,
                     (20 + glm::sin(r) * 10.f) * glm::cos(r)},
                .rotation = glm::identity<glm::quat>()});
        _scene->world().addComponent(id, components::Mesh{cube, mat});
        _scene->world().addComponent(id, components::Wave{0, r});
        _scene->world().addComponent(id, components::Spin{glm::sin(r) * 2.f - 1.f});
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
    _scene->world().interrogateEntityUnsafe(
        ent.id,
        [&components](EntityId entity, ArchetypeId archetype, reflex::TypeInfo const* typeInfo, auto* data) {
            nlohmann::json compEl = nlohmann::json::object();
            reflex::encodeToJsonRaw(compEl, *typeInfo->schema, data);
            components.push_back(std::move(compEl));
        });

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

    _entities[index].id = _scene->world().createEntity();

    if (el.contains("components") && el["components"].is_array()) {
        for (nlohmann::json const& compEl : el["components"]) {
            if (!compEl.contains("$schema") || !compEl["$schema"].is_string()) {
                continue;
            }

            auto const name = compEl["$schema"].get<string_view>();
            reflex::TypeInfo const* const compType = _scene->universe().findComponentByName(name);
            if (compType == nullptr) {
                continue;
            }

            void* const compData = _scene->world().addComponentDefault(_entities[index].id, *compType);
            if (compData == nullptr) {
                continue;
            }

            reflex::decodeFromJsonRaw(compEl, *compType->schema, compData);

            for (reflex::SchemaField const& field : compType->schema->fields) {
                if (field.schema->primitive == reflex::SchemaPrimitive::AssetRef) {
                    auto* const assetHandle =
                        reinterpret_cast<UntypedAssetHandle*>(static_cast<char*>(compData) + field.offset);
                    *assetHandle = assetLoader.loadAssetSync(assetHandle->assetId());
                }
            }
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
