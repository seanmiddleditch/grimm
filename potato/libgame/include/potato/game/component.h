// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

#include "potato/reflex/typeid.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class ComponentTypeBase {
    public:
        virtual ~ComponentTypeBase() = default;

        constexpr uint32 size() const noexcept { return _size; }
        constexpr uint32 align() const noexcept { return _align; }

        virtual zstring_view debugName() const noexcept = 0;

        virtual void construct(void* dest) const = 0;
        virtual void construct(void* dest, void* source) const = 0;
        virtual void destruct(void* dest) const = 0;

        virtual void moveTo(void* dest, void* source) const = 0;

    protected:
        ComponentTypeBase(uint32 size, uint32 align) noexcept : _size(size), _align(align) { }

        uint32 _size = 0;
        uint32 _align = 0;
    };

    template <typename ComponentT>
    class ComponentType : public ComponentTypeBase {
    public:
        ComponentType() noexcept : ComponentTypeBase(sizeof(ComponentT), alignof(ComponentT)) { }

        zstring_view debugName() const noexcept override { return _name.c_str(); }

        void construct(void* dest) const override { new (dest) ComponentT(); }
        void construct(void* dest, void* source) const override {
            new (dest) ComponentT(std::move(*static_cast<ComponentT*>(source)));
        }
        void destruct(void* dest) const override { static_cast<ComponentT*>(dest)->~ComponentT(); }

        void moveTo(void* dest, void* source) const override {
            *static_cast<ComponentT*>(dest) = std::move(*static_cast<ComponentT*>(source));
        }

    private:
        decltype(nameof<ComponentT>()) _name = nameof<ComponentT>();
    };

    template <typename ComponentT>
    consteval ComponentId makeComponentId() noexcept {
        reflex::TypeId const typeId = reflex::makeTypeId<ComponentT>();
        return ComponentId{typeId.raw()};
    }
} // namespace up
