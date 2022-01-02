// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "scene/edit_component.h"

#include "potato/audio/sound_resource.h"
#include "potato/game/common.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/traits.h"
#include "potato/spud/vector.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class Mesh;
    class Material;
    class AssetLoader;
    struct SceneComponent;

    enum class SceneEntityId : uint64 { None = 0 };

    struct SceneEntity {
        string name;
        SceneEntityId sceneId = SceneEntityId::None;
        EntityId previewId = EntityId::None;
        vector<box<SceneComponent>> components;
        int firstChild = -1;
        int nextSibling = -1;
        int parent = -1;
    };

    struct SceneComponent {
        enum class State { Idle, New, Pending, Removed } state = State::New;
        string name;
        SceneEntityId parent = SceneEntityId::None;
        EditComponent const* info = nullptr;
        box<void> data;
    };

    class SceneDatabase {
    public:
        box<SceneComponent> createByName(string_view name);
        box<SceneComponent> createByType(EditComponent const& component);

        deref_span<box<EditComponent> const> components() const noexcept { return _components; }

        template <typename T>
        void registerComponent() {
            _components.push_back(new_box<T>());
        }

    private:
        vector<box<EditComponent>> _components;
    };

    class SceneDocument {
    public:
        SceneDocument(string filename, SceneDatabase& database)
            : _filename(std::move(filename))
            , _database(database) { }

        sequence<int> indices() const noexcept { return sequence{static_cast<int>(_entities.size())}; }
        SceneEntity& entityAt(int index) noexcept { return _entities[index]; }
        int indexOf(SceneEntityId entityId) const noexcept;

        SceneEntityId createEntity(string name, SceneEntityId parentId = SceneEntityId::None);
        void deleteEntity(SceneEntityId targetId);

        void parentTo(SceneEntityId childId, SceneEntityId parentId);

        auto addNewComponent(SceneEntityId entityId, EditComponent const& component) -> SceneComponent*;

        void createTestObjects(Mesh::Handle const& cube, Material::Handle const& mat, SoundHandle const& ding);
        void syncPreview(Space& space);
        void syncGame(Space& space) const;

        void toJson(nlohmann::json& doc) const;
        void fromJson(nlohmann::json const& doc, AssetLoader& assetLoader);

        zstring_view filename() const noexcept { return _filename; }

    private:
        void _deleteEntityAt(int index, vector<SceneEntityId>& out_deleted);
        void _toJson(nlohmann::json& el, int index) const;
        void _fromJson(nlohmann::json const& el, int index, AssetLoader& assetLoader);

        SceneEntityId _allocateEntityId();
        SceneEntityId _consumeEntityId(SceneEntityId entityId);

        string _filename;
        vector<SceneEntity> _entities;
        SceneEntityId _nextEntityId = SceneEntityId{1};
        SceneDatabase& _database;
    };
} // namespace up
