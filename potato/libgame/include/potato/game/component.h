// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/typeid.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    template <typename ComponentT>
    consteval ComponentId makeComponentId() noexcept {
        reflex::TypeId const typeId = reflex::makeTypeId<std::remove_cvref_t<ComponentT>>();
        return ComponentId{typeId.raw()};
    }

    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        constexpr ComponentId componentId() const noexcept { return _id; }

        zstring_view debugName() const noexcept { return _debugName; }

        inline size_t size() const noexcept { return _size; }

        inline void* add(EntityId entityId);
        inline bool remove(EntityId entityId);
        inline [[nodiscard]] bool contains(EntityId entityId) const noexcept;

        inline [[nodiscard]] void* getUnsafe(EntityId entityId) noexcept;

        inline void forEachUnsafe(delegate_ref<bool(EntityId, void*)> callback);

    protected:
        static constexpr uint32 InvalidIndex = uint32(-1);

        explicit ComponentStorage(ComponentId id, zstring_view debugName) noexcept : _id(id), _debugName(debugName) { }

        virtual void* allocateComponentAt(uint32 index) = 0;
        virtual void* getByIndexUnsafe(uint32 index) noexcept = 0;

    private:
        inline [[nodiscard]] uint32 allocateIndex(EntityId entityId);
        inline [[nodiscard]] uint32 indexOf(EntityId entityId) const noexcept;

        vector<EntityId> _entities;
        vector<uint32> _free;
        size_t _size = 0;
        ComponentId _id;
        zstring_view _debugName;
    };

    template <typename ComponentT>
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

    void* ComponentStorage::add(EntityId entityId) {
        {
            uint32 const index = indexOf(entityId);
            if (index != InvalidIndex) {
                return getByIndexUnsafe(index);
            }
        }

        uint32 const index = allocateIndex(entityId);
        ++_size;
        return allocateComponentAt(index);
    }

    bool ComponentStorage::remove(EntityId entityId) {
        if (_size != 0) {
            uint32 const index = indexOf(entityId);
            if (index != InvalidIndex) {
                _entities[index] = {};
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

    void ComponentStorage::forEachUnsafe(delegate_ref<bool(EntityId, void*)> callback) {
        for (uint32 index = 0; index != _entities.size(); ++index) {
            if (_entities[index] != EntityId::None) {
                if (!callback(_entities[index], getByIndexUnsafe(index))) {
                    break;
                }
            }
        }
    }

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
        if (_size != 0 && entityId != EntityId::None) {
            for (uint32 index = 0; index != _entities.size(); ++index) {
                if (_entities[index] == entityId) {
                    return index;
                }
            }
        }
        return InvalidIndex;
    }

    template <typename ComponentT>
    void* TypedComponentStorage<ComponentT>::allocateComponentAt(uint32 index) {
        UP_ASSERT(index <= _components.size());
        return index < _components.size() ? &_components[index] : &_components.emplace_back();
    }

    template <typename ComponentT>
    void* TypedComponentStorage<ComponentT>::getByIndexUnsafe(uint32 index) noexcept {
        UP_ASSERT(index < _components.size());
        return &_components[index];
    }
} // namespace up
