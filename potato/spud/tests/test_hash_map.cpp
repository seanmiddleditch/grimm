// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/hash_map.h"
#include "potato/spud/string.h"

#include <catch2/catch.hpp>

namespace {
    template <typename T>
    struct counted {
        counted(int& counter, T&& value) : _counter(&counter), _value(value) { ++*_counter; }
        ~counted() {
            if (_counter != nullptr) {
                --*_counter;
            }
        }

        counted(counted const& rhs) : _counter(rhs._counter), _value(rhs._value) { ++*_counter; }
        counted(counted&& rhs) noexcept : _counter(rhs._counter), _value(std::move(rhs._value)) {
            rhs._counter = nullptr;
        }

        T* operator->() noexcept { return &_value; }
        T& operator*() noexcept { return _value; }

        counted& operator=(counted const& rhs) {
            if (this != &rhs) {
                if (_counter != nullptr) {
                    --*_counter;
                }

                _counter = rhs._counter;
                ++*_counter;

                _value = rhs._value;
            }
            return *this;
        }

        counted& operator=(counted&& rhs) noexcept {
            if (_counter != nullptr) {
                --*_counter;
            }

            _counter = rhs._counter;
            rhs._counter = nullptr;

            _value = std::move(rhs._value);
            return *this;
        }

        int* _counter = nullptr;
        T _value;
    };
} // namespace

TEST_CASE("potato.spud.hash_map", "[potato][spud]") {
    using namespace up;

    SECTION("fill") {
        hash_map<string, int> values;

        CHECK(values.insert("test", 7));
        CHECK_FALSE(values.insert("test", 8));
        CHECK(values.insert("bob", 1));

        REQUIRE(values.contains("test"));
        REQUIRE(values.contains("bob"));
        CHECK_FALSE(values.contains(""));

        CHECK(values.find("test")->value == 8);
        CHECK(values.find("bob")->value == 1);

        CHECK(2 == values.size());

        // we expect only a single group
        CHECK(16 == values.capacity());
    }

    SECTION("fill large") {
        int counter = 0;
        hash_map<int, counted<int>> values;

        constexpr int count = 3'000; // not a power of 2, intentionally

        for (int i = 0; i != count; ++i) {
            CHECK(values.insert(i, counted(counter, -(i + i))));
        }

        REQUIRE(values.size() == count);
        CHECK(counter == count);

        for (int i = 0; i != count; ++i) {
            REQUIRE(values.contains(i));
            CHECK(*values.find(i)->value == -(i + i));
        }

        values.clear();

        CHECK(counter == 0);
    }

    SECTION("erase") {
        hash_map<int, int> values;

        constexpr int count = 2'048;
        static_assert((count & (count - 1)) == 0);

        for (int i = 0; i != count; ++i) {
            values.insert(i, -(i + i));
        }

        for (int i = 0; i < count; i += 2) {
            REQUIRE(values.erase(i));
        }

        CHECK(values.size() == count / 2);

        for (int i = 1; i < count; i += 2) {
            REQUIRE(values.contains(i));
            CHECK(values.find(i)->value == -(i + i));
        }

        for (int i = 1; i < count; i += 2) {
            values.erase(i);
        }

        CHECK(values.empty());
    }
}
