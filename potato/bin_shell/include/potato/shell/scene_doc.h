// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "scene.h"
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
    class Scene;

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

    template <typename PointerT, typename ValueT = decltype(*declval<PointerT>())>
    class deref_span {
    public:
        using value_type = ValueT;
        using size_type = size_t;
        using reference = ValueT&;

        class iterator {
        public:
            constexpr explicit iterator(PointerT* ptr) noexcept : _ptr(ptr) {}

            constexpr reference operator*() const noexcept { return **_ptr; }
            constexpr iterator& operator++() noexcept {
                ++_ptr;
                return *this;
            }

            constexpr bool operator==(iterator it) const noexcept { return _ptr == it._ptr; }

        private:
            PointerT* _ptr = nullptr;
        };

        template <typename RangeT>
        constexpr deref_span(RangeT&& range) noexcept : _first(range.data())
                                                      , _last(_first + range.size()) {}

        constexpr iterator begin() const noexcept { return iterator(_first); }
        constexpr iterator end() const noexcept { return iterator(_last); }

        constexpr size_type size() const noexcept { return _last - _first; }

    private:
        PointerT* _first = nullptr;
        PointerT* _last = nullptr;
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
        SceneDocument(string filename, SceneDatabase& database) : _filename(std::move(filename)), _database(database) {}

        sequence<int> indices() const noexcept { return sequence{static_cast<int>(_entities.size())}; }
        SceneEntity& entityAt(int index) noexcept { return _entities[index]; }
        int indexOf(SceneEntityId entityId) const noexcept;

        SceneEntityId createEntity(string name, SceneEntityId parentId = SceneEntityId::None);
        void deleteEntity(SceneEntityId targetId);

        void parentTo(SceneEntityId childId, SceneEntityId parentId);

        auto addNewComponent(SceneEntityId entityId, EditComponent const& component) -> SceneComponent*;

        void createTestObjects(Mesh::Handle const& cube, Material::Handle const& mat, SoundHandle const& ding);
        void syncPreview(Scene& scene);
        void syncGame(Scene& scene) const;

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
