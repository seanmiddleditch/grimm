// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/hash.h"
#include "potato/spud/zstring_view.h"

namespace up::reflex {
    struct Schema;

    using FnTypeDefaultConstructor = void (*)(void* memory);
    using FnTypeMoveConstructor = void (*)(void* memory, void* source);
    using FnTypeCopyConstructor = void (*)(void* memory, void const* source);
    using FnTypeMoveAssignment = void (*)(void* target, void* source);
    using FnTypeCopyAssignment = void (*)(void* target, void const* source);
    using FnTypeDestructor = void (*)(void* object);
    using FnTypeRelocater = void (*)(void* memory, void* source);

    struct TypeOps {
        FnTypeDefaultConstructor defaultConstructor = nullptr;
        FnTypeMoveConstructor moveConstructor = nullptr;
        FnTypeCopyConstructor copyConstructor = nullptr;
        FnTypeMoveAssignment moveAssignment = nullptr;
        FnTypeCopyAssignment copyAssignment = nullptr;
        FnTypeDestructor destructor = nullptr;
        FnTypeRelocater relocator = nullptr;
    };

    struct TypeInfo {
        zstring_view name;
        Schema const* schema = nullptr;
        uint64 hash = 0;
        size_t size = 0;
        size_t alignment = 0;
        TypeOps ops;
    };

    template <typename T>
    struct TypeHolder;

    template <typename T>
    constexpr TypeInfo const& getTypeInfo() noexcept {
        return TypeHolder<std::remove_cvref_t<T>>::get();
    }

    template <typename T>
    constexpr TypeOps makeTypeOps() noexcept {
        TypeOps ops;
        if constexpr (std::is_default_constructible_v<T>) {
            ops.defaultConstructor = [](void* memory) {
                new (memory) T{};
            };
        }
        if constexpr (std::is_move_constructible_v<T>) {
            ops.moveConstructor = [](void* memory, void* source) {
                new (memory) T{static_cast<T&&>(*static_cast<T*>(source))};
            };
        }
        if constexpr (std::is_copy_constructible_v<T>) {
            ops.copyConstructor = [](void* memory, void const* source) {
                new (memory) T{*static_cast<T const*>(source)};
            };
        }
        if constexpr (std::is_move_assignable_v<T>) {
            ops.moveAssignment = [](void* target, void* source) {
                *static_cast<T*>(target) = static_cast<T&&>(*static_cast<T*>(source));
            };
        }
        if constexpr (std::is_copy_assignable_v<T>) {
            ops.copyAssignment = [](void* target, void const* source) {
                *static_cast<T*>(target) = *static_cast<T const*>(source);
            };
        }
        ops.destructor = [](void* object) {
            static_cast<T*>(object)->~T();
        };
        if constexpr (std::is_move_constructible_v<T>) {
            ops.relocator = [](void* memory, void* source) {
                new (memory) T{static_cast<T&&>(*static_cast<T*>(source))};
                static_cast<T*>(source)->~T();
            };
        }
        return ops;
    }

    template <typename T>
    constexpr TypeInfo makeTypeInfo(zstring_view name, Schema const* schema) noexcept {
        TypeInfo info;
        info.name = name;
        info.schema = schema;
        info.hash = hash_value(name);
        info.size = sizeof(T);
        info.alignment = alignof(T);
        info.ops = makeTypeOps<T>();
        return info;
    }
} // namespace up::reflex
