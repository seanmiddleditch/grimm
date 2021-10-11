// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>

namespace up {
    template <typename T, std::size_t CacheLineWidth = 64>
    struct alignas(CacheLineWidth) AlignedAtomic : std::atomic<T> {
        using std::atomic<T>::atomic;
        char _padding[CacheLineWidth - sizeof(std::atomic<T>)];
    };

    template <typename T, std::size_t Capacity = 512, std::size_t CacheLineWidth = 64>
    class LockFreeQueue {
        static constexpr std::uint32_t kBufferSize = Capacity;
        static constexpr std::uint32_t kBufferMask = kBufferSize - 1;

        static_assert((kBufferSize & kBufferMask) == 0, "LockFreeQueue size must be a power of 2");

    public:
        LockFreeQueue();
        ~LockFreeQueue();

        LockFreeQueue(LockFreeQueue const&) = delete;
        LockFreeQueue& operator=(LockFreeQueue const&) = delete;

        constexpr auto capacity() const noexcept { return Capacity; }

        template <typename InsertT>
        [[nodiscard]] inline bool tryEnque(InsertT&& value);
        [[nodiscard]] inline bool tryDeque(T& out);

    private:
        AlignedAtomic<std::uint32_t, CacheLineWidth> _enque;
        AlignedAtomic<std::uint32_t, CacheLineWidth> _deque;
        AlignedAtomic<std::uint32_t, CacheLineWidth>* _sequence = nullptr;
        std::aligned_storage_t<sizeof(T), alignof(T)>* _buffer = nullptr;
    };

    template <typename T, std::size_t Capacity, std::size_t CacheLineWidth>
    LockFreeQueue<T, Capacity, CacheLineWidth>::LockFreeQueue()
        : _enque(0)
        , _deque(0)
        , _sequence(new AlignedAtomic<std::uint32_t, CacheLineWidth>[kBufferSize])
        , _buffer(new std::aligned_storage_t<sizeof(T), alignof(T)>[kBufferSize]) {
        for (std::uint32_t i = 0; i != kBufferSize; ++i)
            _sequence[i].store(i, std::memory_order_relaxed);
    }

    template <typename T, std::size_t Capacity, std::size_t CacheLineWidth>
    LockFreeQueue<T, Capacity, CacheLineWidth>::~LockFreeQueue() {
        delete[] _sequence;
        delete[] _buffer;
    }

    template <typename T, std::size_t Capacity, std::size_t CacheLineWidth>
    template <typename InsertT>
    bool LockFreeQueue<T, Capacity, CacheLineWidth>::tryEnque(InsertT&& value) {
        std::uint32_t target = _enque.load(std::memory_order_relaxed);
        std::uint32_t id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
        std::int32_t delta = id - target;

        while (!(delta == 0 && _enque.compare_exchange_weak(target, target + 1, std::memory_order_relaxed))) {
            if (delta < 0) {
                return false;
            }

            target = _enque.load(std::memory_order_relaxed);
            id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
            delta = id - target;
        }

        new (&_buffer[target & kBufferMask]) T(std::forward<InsertT>(value));
        _sequence[target & kBufferMask].store(target + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::size_t Capacity, std::size_t CacheLineWidth>
    bool LockFreeQueue<T, Capacity, CacheLineWidth>::tryDeque(T& out) {
        std::uint32_t target = _deque.load(std::memory_order_relaxed);
        std::uint32_t id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
        std::int32_t delta = id - (target + 1);

        while (!(delta == 0 && _deque.compare_exchange_weak(target, target + 1, std::memory_order_relaxed))) {
            if (delta < 0) {
                return false;
            }

            target = _deque.load(std::memory_order_relaxed);
            id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
            delta = id - (target + 1);
        }

        out = std::move(reinterpret_cast<T&>(_buffer[target & kBufferMask]));
        reinterpret_cast<T&>(_buffer[target & kBufferMask]).~T();
        _sequence[target & kBufferMask].store(target + kBufferMask + 1, std::memory_order_release);
        return true;
    }

} // namespace up
