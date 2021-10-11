// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "const_util.h"
#include "tie_struct.h"
#include "traits.h"
#include "typelist.h"

namespace up {
    namespace _detail {
        // Calculates the sum of sizes in a typelist
        //
        template <typename Types>
        struct sizeof_all;

        template <typename... Types>
        struct sizeof_all<typelist<Types...>> {
            static constexpr int value = static_cast<int>(sum_v<sizeof(Types)...>);
        };

        // Calculates offsets for consecutive items in a typelist
        //
        template <int Stride, int Offset, typename Types, int... Results>
        struct layout_calculator;

        template <int Stride, int Offset, typename Next, typename... Types, int... Results>
        struct layout_calculator<Stride, Offset, typelist<Next, Types...>, Results...> {
            static constexpr int aligned = static_cast<int>(align_to(Offset, alignof(Next)));
            static constexpr int span = static_cast<int>(sizeof(Next) * Stride);
            static constexpr int end = aligned + span;
            using result =
                typename layout_calculator<Stride, aligned + span, typelist<Types...>, Results..., aligned>::result;
        };

        template <int Stride, int Offset, int... Results>
        struct layout_calculator<Stride, Offset, typelist<>, Results...> {
            using result = value_list<int, Results...>;
        };

        template <int Stride, typename Types>
        using layout_calculator_t = typename layout_calculator<Stride, 0, Types>::result;
    } // namespace _detail

    // Provides a mechanism for decoding a struct of pointers into a memory block of size ChunkSize.
    //
    // Note: the provided structure must only contain pointer-type members.
    //
    template <typename Struct, int ChunkSize = 64 * 1024>
    struct soa_layout {
        using type = Struct;
        using member_types = typelist_map_t<std::remove_pointer_t, member_typelist_t<Struct>>;

        static constexpr int chunk_size = ChunkSize;
        static constexpr int stride = chunk_size / (1 + _detail::sizeof_all<member_types>::value);

        using offsets = _detail::layout_calculator_t<stride, member_types>;

        static constexpr auto get(char* data) noexcept -> Struct {
            return _get(data, member_types(), std::make_integer_sequence<int, typelist_size_v<member_types>>());
        }

        template <typename... Types, int... Indices>
        static constexpr auto _get(char* data, typelist<Types...>, std::integer_sequence<int, Indices...>) noexcept
            -> Struct {
            Struct result{static_cast<Types*>(static_cast<void*>(data + value_at_v<Indices, offsets>))...};
            return result;
        }
    };
} // namespace up
