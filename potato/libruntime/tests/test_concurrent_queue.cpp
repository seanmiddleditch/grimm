// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/concurrent_queue.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("potato.runtime.ConcurrentQueue", "[potato][runtime]") {
    using namespace up;

    SECTION("default") { ConcurrentQueue<int> queue; }

    SECTION("thread") {
        ConcurrentQueue<int> queue;

        int last = 0;
        auto consumer = std::thread([&] {
            int value = 0;
            while (queue.dequeWait(value)) {
                last = value;
            }
        });

        auto producer = std::thread([&] {
            for (int i = 0; i != 1024; ++i) {
                queue.enqueWait(i);
            }
            queue.close();
        });
        producer.detach();

        consumer.join();

        CHECK(queue.isClosed());

        CHECK(last == 1023);
    }
}
