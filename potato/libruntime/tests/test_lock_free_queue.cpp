// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/lock_free_queue.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("potato.runtime.LockFreeQueue", "[potato][runtime]") {
    using namespace up;

    SECTION("default") { LockFreeQueue<int> queue; }

    SECTION("fill") {
        LockFreeQueue<std::size_t> queue;

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            CHECK(queue.tryEnque(i));
        }

        CHECK(!queue.tryEnque(0));
    }

    SECTION("sequential") {
        LockFreeQueue<std::size_t> queue;

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            CHECK(queue.tryEnque(i));
        }

        CHECK(!queue.tryEnque(0));

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            std::size_t result = 0;
            REQUIRE(queue.tryDeque(result));
            CHECK(i == result);
        }

        std::size_t empty = 0;
        CHECK(!queue.tryDeque(empty));
    }

    SECTION("thread") {
        LockFreeQueue<std::size_t> queue;

        std::size_t total1 = 0;
        auto thread1 = std::thread([&] {
            for (;;) {
                std::size_t count = 0;
                if (queue.tryDeque(count)) {
                    if (count == 0) {
                        break;
                    }
                    total1 += count;
                }
            }
        });

        std::size_t total2 = 0;
        auto thread2 = std::thread([&] {
            for (;;) {
                std::size_t count = 0;
                if (queue.tryDeque(count)) {
                    if (count == 0) {
                        break;
                    }
                    total2 += count;
                }
            }
        });

        std::size_t expected = 0;
        for (std::size_t i = 2; i < queue.capacity(); ++i) {
            REQUIRE(queue.tryEnque(i));
            expected += i;
        }

        // signals end to threads
        CHECK(queue.tryEnque(0));
        CHECK(queue.tryEnque(0));

        thread1.join();
        thread2.join();

        CHECK(total1 + total2 == expected);
    }
}
