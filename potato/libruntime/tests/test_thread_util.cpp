// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/thread_util.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("potato.runtime.thread_util", "[potato][runtime]") {
    using namespace up;

    SECTION("setCurrentThreadName") {
        // basically just making sure we're not crashing
        auto thread = std::thread([] { setCurrentThreadName("test thread"); });

        thread.join();
    }

    SECTION("currentThreadSmallId") {
        CHECK(currentSmallThreadId() == 0);

        // Yes, this could be a lambda, but I'm seeing a bad miscompilation bug in MSVC 16.6 Preview 5
        // when using lambdas here. These threads end up invoking the lambdas in test_concurrent_queue,
        // and vice versa that file invokes these lambdas. Naturally, the captured state and environment
        // are totally wrong and at best we get assert failures and at worst we get crashes.
        //
        struct Task {
            SmallThreadId& id;
            Task(SmallThreadId& i) : id(i) {}
            void operator()() const { CHECK((id = currentSmallThreadId()) != 0); }
        };

        // ensure it doesn't change
        REQUIRE(currentSmallThreadId() == 0);

        SmallThreadId id1 = 0;
        Task t1(id1);
        auto thread = std::thread(t1);
        thread.join();

        SmallThreadId id2 = 0;
        Task t2(id2);
        thread = std::thread(t2);
        thread.join();

        CHECK(id1 != id2);

        // other threads shouldn't have changed
        CHECK(currentSmallThreadId() == 0);
    }
}
