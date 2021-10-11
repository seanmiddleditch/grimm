// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_assertion.h"
#include "int_types.h"

#include <cstring>

namespace up {
    class bit_set {
    public:
        using index_type = size_t;
        using size_type = size_t;

        bit_set() = default;
        inline explicit bit_set(size_type capacity);
        inline ~bit_set();

        bit_set(bit_set const&) = delete;
        bit_set& operator=(bit_set const&) = delete;

        inline size_type capacity() const noexcept { return _elemSize * bits; }

        inline bit_set(bit_set&& rhs) noexcept;
        inline bit_set& operator=(bit_set&&) noexcept;

        inline bool test(index_type index) const noexcept;
        inline bit_set& set(index_type index);
        inline bit_set& reset(index_type index) noexcept;

        inline bool operator==(bit_set const& rhs) const noexcept;
        inline bool has_all(bit_set const& rhs) const noexcept;

        inline bit_set clone() const;
        inline bit_set& resize(size_type capacity);

    private:
        using element = uint64;
        static constexpr size_t bits = sizeof(element) * 8;

        element* _elems = nullptr;
        size_type _elemSize = 0;
    };

    bit_set::bit_set(size_type capacity) { resize(capacity); }

    bit_set::~bit_set() { delete[] _elems; }

    bit_set::bit_set(bit_set&& rhs) noexcept : _elems(rhs._elems), _elemSize(rhs._elemSize) {
        rhs._elems = nullptr;
        rhs._elemSize = 0;
    }

    bit_set& bit_set::operator=(bit_set&& rhs) noexcept {
        UP_SPUD_ASSERT(this != &rhs, "cannot move a bit_set over itself");
        _elems = rhs._elems;
        _elemSize = rhs._elemSize;
        rhs._elems = nullptr;
        rhs._elemSize = 0;
        return *this;
    }

    bool bit_set::test(index_type index) const noexcept {
        if (index >= _elemSize * bits) {
            return false;
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        return (data & mask) != 0;
    }

    bit_set& bit_set::set(index_type index) {
        if (index >= _elemSize * bits) {
            resize(index + 1);
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        data |= mask;
        return *this;
    }

    bit_set& bit_set::reset(index_type index) noexcept {
        if (index >= _elemSize * bits) {
            return *this;
        }
        size_t elem = index / bits;
        size_t bit = index - (elem * bits);
        element mask = element(1) << bit;
        element& data = _elems[elem];
        data &= ~mask;
        return *this;
    }

    bool bit_set::operator==(bit_set const& rhs) const noexcept {
        size_type common = _elemSize > rhs._elemSize ? rhs._elemSize : _elemSize;
        for (size_type index = 0; index != common; ++index) {
            element& left = _elems[index];
            element& right = rhs._elems[index];
            if (left != right) {
                return false;
            }
        }

        bit_set const& remainder = _elemSize > rhs._elemSize ? *this : rhs;
        for (size_t index = common; index != remainder._elemSize; ++index) {
            element& el = remainder._elems[index];
            if (el != element(0)) {
                return false;
            }
        }

        return true;
    }

    bool bit_set::has_all(bit_set const& rhs) const noexcept {
        size_type common = _elemSize > rhs._elemSize ? rhs._elemSize : _elemSize;
        for (size_type index = 0; index < common; ++index) {
            element& left = _elems[index];
            element& right = rhs._elems[index];
            if ((left & right) != right) {
                return false;
            }
        }

        for (size_t index = common; index < rhs._elemSize; ++index) {
            element& el = rhs._elems[index];
            if (el != element(0)) {
                return false;
            }
        }

        return true;
    }

    bit_set bit_set::clone() const {
        bit_set rs;
        rs._elemSize = _elemSize;
        rs._elems = new element[_elemSize];
        std::memcpy(rs._elems, _elems, _elemSize * sizeof(element));
        return rs;
    }

    bit_set& bit_set::resize(size_type capacity) {
        size_t const existingCapacity = _elemSize * bits;
        if (capacity == existingCapacity) {
            return *this;
        }

        size_t const newSize = (capacity + bits - 1) / bits;
        auto* newElems = new element[newSize];

        if (capacity > existingCapacity) {
            std::memcpy(newElems, _elems, _elemSize * sizeof(element));
            std::memset(newElems + _elemSize, 0, (newSize - _elemSize) * sizeof(element));
        }
        else if (capacity < existingCapacity) {
            std::memcpy(newElems, _elems, newSize * sizeof(element));
        }

        delete[] _elems;

        _elems = newElems;
        _elemSize = newSize;

        return *this;
    }
} // namespace up
