// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "scene.h"

#include "potato/audio/sound_resource.h"
#include "potato/ecs/common.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class Mesh;
    class Material;
    class AssetLoader;

    struct SceneEntity {
        string name;
        EntityId id = EntityId::None;
        int firstChild = -1;
        int nextSibling = -1;
        int parent = -1;
    };

    class SceneDocument {
    public:
        SceneDocument(string filename, rc<Scene> scene) : _filename(std::move(filename)), _scene(std::move(scene)) {}

        rc<Scene> const& scene() const { return _scene; }

        sequence<int> indices() const noexcept { return sequence{static_cast<int>(_entities.size())}; }
        SceneEntity& entityAt(int index) noexcept { return _entities[index]; }
        int indexOf(EntityId entityId) const noexcept;

        EntityId createEntity(string name, EntityId parentId = EntityId::None);
        void deleteEntity(EntityId targetId);

        void parentTo(EntityId childId, EntityId parentId);

        void createTestObjects(Mesh::Handle const& cube, Material::Handle const& mat, SoundHandle const& ding);

        void toJson(nlohmann::json& doc) const;
        void fromJson(nlohmann::json const& doc, AssetLoader& assetLoader);

        zstring_view filename() const noexcept { return _filename; }

    private:
        void _deleteEntityAt(int index, vector<EntityId>& out_deleted);
        void _toJson(nlohmann::json& el, int index) const;
        void _fromJson(nlohmann::json const& el, int index, AssetLoader& assetLoader);

        string _filename;
        rc<Scene> _scene;
        vector<SceneEntity> _entities;
    };
} // namespace up
