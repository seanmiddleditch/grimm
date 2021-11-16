// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/typeid.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/find.h"
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
        [[nodiscard]] EntityId entityId() const noexcept { return _entityId; }
        [[nodiscard]] void* componentData() const noexcept { return _componentData; }

        [[nodiscard]] inline bool next() noexcept;

    private:
        explicit ComponentCursor(ComponentStorage& storage) : _storage(&storage) { }

        ComponentStorage* _storage = nullptr;
        EntityId _entityId = EntityId::None;
        void* _componentData = nullptr;
        uint32 _index = 0;

        friend ComponentStorage;
    };

    class RawComponentObserver {
    public:
        virtual ComponentId componentId() const = 0;
        virtual void onAdd(EntityId entityId, void* data) = 0;
        virtual void onRemove(EntityId entityId, void* data) = 0;

    protected:
        ~RawComponentObserver() = default;
    };

    template <typename ComponentT>
    class ComponentObserver : public RawComponentObserver {
    public:
        ComponentId componentId() const final { return makeComponentId<ComponentT>(); }

        virtual void onAdd(EntityId entityId, ComponentT& component) = 0;
        virtual void onRemove(EntityId entityId, ComponentT& component) = 0;

    protected:
        ~ComponentObserver() = default;

    private:
        void onAdd(EntityId entityId, void* data) final { onAdd(entityId, *static_cast<ComponentT*>(data)); }
        void onRemove(EntityId entityId, void* data) final { onRemove(entityId, *static_cast<ComponentT*>(data)); }
    };

    class ComponentStorage {
    public:
        virtual ~ComponentStorage() = default;

        [[nodiscard]] constexpr ComponentId componentId() const noexcept { return _id; }
        [[nodiscard]] virtual zstring_view debugName() const noexcept = 0;

        [[nodiscard]] size_t size() const noexcept { return _size; }

        inline void* add(EntityId entityId, void const* source);
        inline bool remove(EntityId entityId);
        [[nodiscard]] inline bool contains(EntityId entityId) const noexcept;

        [[nodiscard]] inline void* getUnsafe(EntityId entityId) noexcept;

        [[nodiscard]] inline ComponentCursor enumerateUnsafe() noexcept;

        inline void observe(RawComponentObserver* observer);
        inline void unobserve(RawComponentObserver* observer);

    protected:
        static constexpr uint32 InvalidIndex = uint32(-1);

        explicit ComponentStorage(ComponentId id) noexcept : _id(id) { }

        virtual void* allocateComponentAt(uint32 index, void const* source) = 0;
        virtual void* getByIndexUnsafe(uint32 index) noexcept = 0;

    private:
        [[nodiscard]] inline uint32 allocateIndex(EntityId entityId);
        [[nodiscard]] inline uint32 indexOf(EntityId entityId) const noexcept;

        vector<EntityId> _entities;
        vector<uint32> _free;
        vector<RawComponentObserver*> _observers;
        hash_map<EntityId, uint32> _map;
        size_t _size = 0;
        ComponentId _id;

        friend ComponentCursor;
    };

    template <typename ComponentT, bool IsEmptyComponent = false>
    class TypedComponentStorage final : public ComponentStorage {
    public:
        TypedComponentStorage() noexcept
            : ComponentStorage(makeComponentId<ComponentT>())
            , _name(nameof<ComponentT>()) { }

    private:
        zstring_view debugName() const noexcept override { return _name.c_str(); }

        void* allocateComponentAt(uint32 index, void const* source) override;
        void* getByIndexUnsafe(uint32 index) noexcept override;

        vector<ComponentT> _components;
        decltype(nameof<ComponentT>()) _name;
    };

    template <typename ComponentT>
    class TypedComponentStorage<ComponentT, true> final : public ComponentStorage {
    public:
        TypedComponentStorage() noexcept
            : ComponentStorage(makeComponentId<ComponentT>())
            , _name(nameof<ComponentT>()) { }

    private:
        zstring_view debugName() const noexcept override { return _name.c_str(); }

        void* allocateComponentAt(uint32 index, void const*) override { return &_empty; }
        void* getByIndexUnsafe(uint32 index) noexcept override { return &_empty; }

        decltype(nameof<ComponentT>()) _name;
        ComponentT _empty;
    };

    bool ComponentCursor::next() noexcept {
        while (_index < _storage->_entities.size()) {
            uint32 const index = _index++;
            EntityId const entityId = _storage->_entities[index];
            if (entityId != EntityId::None) {
                _entityId = entityId;
                _componentData = _storage->getByIndexUnsafe(index);
                return true;
            }
        }
        return false;
    }

    void* ComponentStorage::add(EntityId entityId, void const* source) {
        {
            uint32 const index = indexOf(entityId);
            if (index != InvalidIndex) {
                return getByIndexUnsafe(index);
            }
        }

        uint32 const index = allocateIndex(entityId);
        _map.insert(entityId, index);
        ++_size;
        void* component = allocateComponentAt(index, source);

        for (RawComponentObserver* observer : _observers) {
            observer->onAdd(entityId, component);
        }

        return component;
    }

    bool ComponentStorage::remove(EntityId entityId) {
        if (_size == 0) {
            return false;
        }

        uint32 const index = indexOf(entityId);
        if (index == InvalidIndex) {
            return false;
        }

        for (RawComponentObserver* observer : _observers) {
            observer->onRemove(entityId, getByIndexUnsafe(index));
        }

        _entities[index] = {};
        _map.erase(entityId);
        _free.push_back(index);
        --_size;
        return true;
    }

    bool ComponentStorage::contains(EntityId entityId) const noexcept {
        return _size != 0 ? indexOf(entityId) != InvalidIndex : false;
    }

    void* ComponentStorage::getUnsafe(EntityId entityId) noexcept {
        uint32 const index = indexOf(entityId);
        return index != InvalidIndex ? getByIndexUnsafe(index) : nullptr;
    }

    ComponentCursor ComponentStorage::enumerateUnsafe() noexcept { return ComponentCursor{*this}; }

    void ComponentStorage::observe(RawComponentObserver* observer) {
        UP_GUARD_VOID(observer != nullptr);
        UP_GUARD_VOID(observer->componentId() == _id);
        _observers.push_back(observer);
    }

    void ComponentStorage::unobserve(RawComponentObserver* observer) {
        UP_GUARD_VOID(observer != nullptr);
        UP_GUARD_VOID(observer->componentId() == _id);
        size_t const index = find(_observers, observer) - _observers.begin();
        UP_GUARD_VOID(index != _observers.size());
        _observers[index] = _observers.back();
        _observers.pop_back();
    }

    [[nodiscard]] uint32 ComponentStorage::allocateIndex(EntityId entityId) {
        if (!_free.empty()) {
            uint32 const index = _free.back();
            _entities[index] = entityId;
            _free.pop_back();
            return index;
        }

        auto const index = static_cast<uint32>(_entities.size());
        _entities.push_back(entityId);
        return index;
    }

    uint32 ComponentStorage::indexOf(EntityId entityId) const noexcept {
        auto const& item = _map.find(entityId);
        return item ? item->value : InvalidIndex;
    }

    template <typename ComponentT, bool IsEmptyComponent>
    void* TypedComponentStorage<ComponentT, IsEmptyComponent>::allocateComponentAt(uint32 index, void const* source) {
        UP_ASSERT(index <= _components.size());
        if (source != nullptr) {
            if (index == _components.size()) {
                _components.emplace_back(*static_cast<ComponentT const*>(source));
            }
            else {
                _components[index] = *static_cast<ComponentT const*>(source);
            }
        }
        else {
            if (index == _components.size()) {
                _components.emplace_back();
            }
            else {
                _components[index] = ComponentT{};
            }
        }
        return &_components[index];
    }

    template <typename ComponentT, bool IsEmptyComponent>
    void* TypedComponentStorage<ComponentT, IsEmptyComponent>::getByIndexUnsafe(uint32 index) noexcept {
        UP_ASSERT(index < _components.size());
        return &_components[index];
    }
} // namespace up
