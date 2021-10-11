// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"

#include <atomic>
#include <thread>

namespace up {
    class Spinlock {
    public:
        inline void lock() noexcept;
        [[nodiscard]] inline bool tryLock() noexcept;
        [[nodiscard]] inline bool isLocked() const noexcept;
        inline void unlock() noexcept;

    private:
        std::atomic<std::thread::id> _owner = std::thread::id();
    };

    void Spinlock::lock() noexcept {
        // try to acquire the lock
        // FIXME - exponential backoff should be added
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        while (!_owner.compare_exchange_weak(expected, desired, std::memory_order_acquire)) {
            expected = std::thread::id();
        }
    }

    bool Spinlock::tryLock() noexcept {
        // try to acquire the lock
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        return _owner.compare_exchange_strong(expected, desired, std::memory_order_acquire);
    }

    bool Spinlock::isLocked() const noexcept { return _owner != std::thread::id(); }

    void Spinlock::unlock() noexcept {
        UP_ASSERT(_owner == std::this_thread::get_id());

        // release the lock
        _owner.store(std::thread::id(), std::memory_order_release);
    }

} // namespace up
