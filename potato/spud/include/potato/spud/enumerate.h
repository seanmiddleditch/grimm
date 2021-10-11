// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"

#include <type_traits>

namespace up {
    template <typename RangeT>
    class enumerate;

    template <typename RangeT>
    enumerate(RangeT&&) -> enumerate<RangeT>;
    template <typename RangeT>
    enumerate(RangeT&) -> enumerate<RangeT>;
    template <typename RangeT>
    enumerate(RangeT const&) -> enumerate<RangeT const>;

    namespace _detail::enumerate {
        template <typename SizeT, typename ReferenceT>
        struct entry {
            SizeT index;
            ReferenceT item;
        };

        template <typename SentinelT>
        struct sentinel {
            SentinelT end;
        };

        template <typename SizeT, typename IteratorT, typename SentinelT>
        class iterator {
        public:
            iterator() noexcept = default;

            using reference = std::add_lvalue_reference_t<decltype(*std::declval<IteratorT>())>;
            using value_type = entry<SizeT, reference>;

            constexpr auto operator*() const noexcept -> value_type { return {_index, *_iterator}; }

            constexpr auto operator++() noexcept -> iterator& {
                ++_iterator;
                ++_index;
                return *this;
            }

            constexpr auto operator++(int) noexcept -> iterator {
                auto tmp = *this;
                ++_iterator;
                ++_index;
                return tmp;
            }

            friend bool operator==(iterator const& lhs, iterator const& rhs) noexcept = default;
            friend constexpr bool operator==(iterator const& it, sentinel<SentinelT> const& se) noexcept {
                return it._iterator == se.end;
            }

        private:
            constexpr iterator(IteratorT iterator) noexcept : _iterator(iterator) {}

            template <typename>
            friend class up::enumerate;

            IteratorT _iterator;
            SizeT _index = {};
        };
    } // namespace _detail::enumerate

    template <typename RangeT>
    class enumerate {
    private:
        using range_iterator =
            std::conditional_t<std::is_const_v<RangeT>, typename RangeT::const_iterator, typename RangeT::iterator>;
        using range_sentinel = range_iterator;

    public:
        using iterator = _detail::enumerate::iterator<typename RangeT::size_type, range_iterator, range_sentinel>;
        using sentinel = _detail::enumerate::sentinel<range_sentinel>;
        using value_type = typename iterator::value_type;

        constexpr explicit enumerate(RangeT& range) noexcept : _range(range) {}
        constexpr explicit enumerate(RangeT&& range) noexcept : _range(range) {}

        constexpr auto begin() const noexcept -> iterator { return iterator{_range.begin()}; }
        constexpr auto end() const noexcept -> sentinel { return sentinel{_range.end()}; }

    private:
        RangeT& _range;
    };

} // namespace up
