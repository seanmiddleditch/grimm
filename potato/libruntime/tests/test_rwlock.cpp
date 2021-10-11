// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/rwlock.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("potato.runtime.RWLock", "[potato][runtime]") {
    using namespace up;

    SECTION("writer-reader-writer") {
        RWLock lock;

        lock.writer().lock();
        CHECK_FALSE(lock.writer().tryLock());
        CHECK_FALSE(lock.reader().tryLock());
        lock.writer().unlock();

        lock.reader().lock();
        CHECK_FALSE(lock.writer().tryLock());
        lock.reader().lock();
        CHECK_FALSE(lock.writer().tryLock());
        lock.reader().unlock();
        CHECK_FALSE(lock.writer().tryLock());
        lock.reader().unlock();

        lock.writer().lock();
        CHECK_FALSE(lock.writer().tryLock());
        CHECK_FALSE(lock.reader().tryLock());
        lock.writer().unlock();
    }
}
