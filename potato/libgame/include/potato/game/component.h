// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/typeid.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        constexpr ComponentId componentId() const noexcept { return _id; }

        virtual zstring_view debugName() const noexcept = 0;

        virtual void* add(EntityId entity) = 0;
        virtual bool remove(EntityId entity) = 0;
        virtual [[nodiscard]] bool contains(EntityId entity) const noexcept = 0;

        virtual [[nodiscard]] void* getUnsafe(EntityId entity) noexcept = 0;

        virtual void forEachUnsafe(delegate_ref<bool(EntityId, void*)> callback) = 0;

    protected:
        explicit ComponentStorage(ComponentId id) noexcept : _id(id) { }

    private:
        ComponentId _id;
    };

    template <typename ComponentT>
    class TypedComponentStorage : public ComponentStorage {
    public:
        TypedComponentStorage() noexcept : ComponentStorage(makeComponentId<ComponentT>()) { }

        zstring_view debugName() const noexcept override { return _name.c_str(); }

        void* add(EntityId entityId) override {
            {
                uint32 const index = _indexOf(entityId);
                if (index != _entities.size()) {
                    return &_components[index];
                }
            }

            if (!_free.empty()) {
                uint32 const index = _free.back();
                _entities[index] = entityId;
                _free.pop_back();
                return &_components[index];
            }

            _entities.push_back(entityId);
            return &_components.emplace_back();
        }

        bool remove(EntityId entityId) override {
            uint32 const index = _indexOf(entityId);
            if (index != _entities.size()) {
                _entities[index] = {};
                _free.push_back(index);
                return true;
            }
            return false;
        }

        [[nodiscard]] bool contains(EntityId entityId) const noexcept override {
            return _indexOf(entityId) != _entities.size();
        }

        [[nodiscard]] void* getUnsafe(EntityId entityId) noexcept override {
            uint32 const index = _indexOf(entityId);
            return index < _entities.size() ? &_components[index] : nullptr;
        }

        void forEachUnsafe(delegate_ref<bool(EntityId, void*)> callback) override {
            for (size_t index = 0; index != _entities.size(); ++index) {
                if (_entities[index] != EntityId::None) {
                    if (!callback(_entities[index], &_components[index])) {
                        break;
                    }
                }
            }
        }

    private:
        [[nodiscard]] uint32 _indexOf(EntityId entity) const noexcept;

        decltype(nameof<ComponentT>()) _name = nameof<ComponentT>();

        vector<EntityId> _entities;
        vector<ComponentT> _components;
        vector<uint32> _free;
    };

    template <typename ComponentT>
    uint32 TypedComponentStorage<ComponentT>::_indexOf(EntityId entity) const noexcept {
        if (entity != EntityId::None) {
            for (uint32 index = 0; index != _entities.size(); ++index) {
                if (_entities[index] == entity) {
                    return index;
                }
            }
        }
        return static_cast<uint32>(_entities.size());
    }

    template <typename ComponentT>
    consteval ComponentId makeComponentId() noexcept {
        reflex::TypeId const typeId = reflex::makeTypeId<ComponentT>();
        return ComponentId{typeId.raw()};
    }
} // namespace up
