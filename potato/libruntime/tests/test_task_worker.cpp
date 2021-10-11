// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/task_worker.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("potato.runtime.TaskWorker", "[potato][runtime]") {
    using namespace up;

    SECTION("single") {
        TaskQueue queue;
        TaskWorker worker1(queue, "Test Worker 1");
        TaskWorker worker2(queue, "Test Worker 2");

        int values[1024] = {};
        for (int i = 0; i != 1024; ++i) {
            queue.enqueWait([&values, i] { values[i] = i; });
        }

        queue.close();
        worker1.join();
        worker2.join();

        for (int i = 0; i != 1024; ++i) {
            CHECK(values[i] == i);
        }
    }
}
