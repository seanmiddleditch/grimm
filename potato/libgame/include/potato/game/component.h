// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/typeid.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class ComponentStorage;

    template <typename ComponentT>
    consteval ComponentId makeComponentId() noexcept {
        reflex::TypeId const typeId = reflex::makeTypeId<std::remove_cvref_t<ComponentT>>();
        return ComponentId{typeId.raw()};
    }

    class ComponentCursor {
    public:
        EntityId entityId = EntityId::None;
        void* componentData = nullptr;

        [[nodiscard]] inline bool next() noexcept;

    private:
        explicit ComponentCursor(ComponentStorage& storage) : _storage(&storage) { }

        ComponentStorage* _storage = nullptr;
        uint32 _index = 0;

        friend ComponentStorage;
    };

    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        constexpr ComponentId componentId() const noexcept { return _id; }
        zstring_view debugName() const noexcept { return _debugName; }

        size_t size() const noexcept { return _size; }

        inline void* add(EntityId entityId);
        inline bool remove(EntityId entityId);
        [[nodiscard]] inline bool contains(EntityId entityId) const noexcept;

        [[nodiscard]] inline void* getUnsafe(EntityId entityId) noexcept;

        [[nodiscard]] inline ComponentCursor enumerateUnsafe() noexcept;

    protected:
        static constexpr uint32 InvalidIndex = uint32(-1);

        explicit ComponentStorage(ComponentId id, zstring_view debugName) noexcept : _id(id), _debugName(debugName) { }

        virtual void* allocateComponentAt(uint32 index) = 0;
        virtual void* getByIndexUnsafe(uint32 index) noexcept = 0;

    private:
        [[nodiscard]] inline uint32 allocateIndex(EntityId entityId);
        [[nodiscard]] inline uint32 indexOf(EntityId entityId) const noexcept;

        vector<EntityId> _entities;
        vector<uint32> _free;
        hash_map<EntityId, uint32> _map;
        size_t _size = 0;
        ComponentId _id;
        zstring_view _debugName;

        friend ComponentCursor;
    };

    template <typename ComponentT, bool IsEmptyComponent = false>
    class TypedComponentStorage final : public ComponentStorage {
    public:
        TypedComponentStorage() noexcept
            : _name(nameof<ComponentT>())
            , ComponentStorage(makeComponentId<ComponentT>(), _name.c_str()) { }

    private:
        void* allocateComponentAt(uint32 index) override;
        void* getByIndexUnsafe(uint32 index) noexcept override;

        vector<ComponentT> _components;
        decltype(nameof<ComponentT>()) _name;
    };

    template <typename ComponentT>
    class TypedComponentStorage<ComponentT, true> final : public ComponentStorage {
        TypedComponentStorage() noexcept
            : _name(nameof<ComponentT>())
            , ComponentStorage(makeComponentId<ComponentT>(), _name.c_str()) { }

    private:
        void* allocateComponentAt(uint32 index) override { return &_empty; }
        void* getByIndexUnsafe(uint32 index) noexcept override { return &_empty; }

        decltype(nameof<ComponentT>()) _name;
        ComponentT _empty;
    };

    bool ComponentCursor::next() noexcept {
        while (_index < _storage->_entities.size()) {
            uint32 const index = _index++;
            EntityId const entityId = _storage->_entities[index];
            if (entityId != EntityId::None) {
                this->entityId = entityId;
                this->componentData = _storage->getByIndexUnsafe(index);
                return true;
            }
        }
        return false;
    }

    void* ComponentStorage::add(EntityId entityId) {
        {
            uint32 const index = indexOf(entityId);
            if (index != InvalidIndex) {
                return getByIndexUnsafe(index);
            }
        }

        uint32 const index = allocateIndex(entityId);
        _map.insert(entityId, index);
        ++_size;
        return allocateComponentAt(index);
    }

    bool ComponentStorage::remove(EntityId entityId) {
        if (_size != 0) {
            uint32 const index = indexOf(entityId);
            if (index != InvalidIndex) {
                _entities[index] = {};
                _map.erase(entityId);
                _free.push_back(index);
                --_size;
                return true;
            }
        }
        return false;
    }

    bool ComponentStorage::contains(EntityId entityId) const noexcept {
        return _size != 0 ? indexOf(entityId) != InvalidIndex : false;
    }

    void* ComponentStorage::getUnsafe(EntityId entityId) noexcept {
        uint32 const index = indexOf(entityId);
        return index != InvalidIndex ? getByIndexUnsafe(index) : nullptr;
    }

    ComponentCursor ComponentStorage::enumerateUnsafe() noexcept { return ComponentCursor{*this}; }

    [[nodiscard]] uint32 ComponentStorage::allocateIndex(EntityId entityId) {
        if (!_free.empty()) {
            uint32 const index = _free.back();
            _entities[index] = entityId;
            _free.pop_back();
            return index;
        }

        uint32 const index = static_cast<uint32>(_entities.size());
        _entities.push_back(entityId);
        return index;
    }

    uint32 ComponentStorage::indexOf(EntityId entityId) const noexcept {
        auto const& item = _map.find(entityId);
        return item ? item->value : InvalidIndex;
    }

    template <typename ComponentT, bool IsEmptyComponent>
    void* TypedComponentStorage<ComponentT, IsEmptyComponent>::allocateComponentAt(uint32 index) {
        UP_ASSERT(index <= _components.size());
        return index < _components.size() ? &_components[index] : &_components.emplace_back();
    }

    template <typename ComponentT, bool IsEmptyComponent>
    void* TypedComponentStorage<ComponentT, IsEmptyComponent>::getByIndexUnsafe(uint32 index) noexcept {
        UP_ASSERT(index < _components.size());
        return &_components[index];
    }
} // namespace up
