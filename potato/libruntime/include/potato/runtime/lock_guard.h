// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"

#include <atomic>
#include <thread>

namespace up {
    enum class AdoptLock { Adopt };

    template <typename LockT>
    class LockGuard {
    public:
        explicit constexpr LockGuard(LockT& lock) noexcept : _lock(lock) { _lock.lock(); }
        explicit constexpr LockGuard(LockT& lock, AdoptLock) noexcept : _lock(lock) {}
        ~LockGuard() noexcept { _lock.unlock(); }

        LockGuard(LockGuard const&) = delete;
        LockGuard& operator=(LockGuard const&) = delete;

    private:
        LockT& _lock;
    };

    template <typename LockT>
    LockGuard(LockT&) -> LockGuard<LockT>;
} // namespace up
