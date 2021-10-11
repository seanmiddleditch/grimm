// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace up {
    template <typename T>
    class ConcurrentQueue {
    public:
        static constexpr int default_capacity = 1024;

        ConcurrentQueue() : ConcurrentQueue(default_capacity) {}
        explicit ConcurrentQueue(uint32 capacity);
        ~ConcurrentQueue();

        ConcurrentQueue(ConcurrentQueue const&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;

        void close();
        void waitUntilEmpty();

        bool isClosed() noexcept;

        template <typename InsertT>
        [[nodiscard]] bool tryEnque(InsertT&& value);
        template <typename InsertT>
        [[nodiscard]] bool tryEnque(InsertT const& value);
        [[nodiscard]] bool tryDeque(T& out);

        template <typename InsertT>
        void enqueWait(InsertT&& value);
        template <typename InsertT>
        void enqueWait(InsertT const& value);
        [[nodiscard]] bool dequeWait(T& out);

    private:
        void _grow();

        using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

        std::mutex _lock;
        std::condition_variable _condition;
        bool _closed = false;
        uint32 _start = 0;
        uint32 _size = 0;
        uint32 _mask = 0;
        storage* _buffer = nullptr;
    };

    template <typename T>
    ConcurrentQueue<T>::ConcurrentQueue(uint32 capacity) : _mask(capacity - 1)
                                                         , _buffer(new storage[capacity]) {
        UP_ASSERT((capacity & _mask) == 0, "ConcurrentQueue capacity must be a power-of-two");
    }

    template <typename T>
    ConcurrentQueue<T>::~ConcurrentQueue() {
        close();
        waitUntilEmpty();
        delete[] _buffer;
    }

    template <typename T>
    void ConcurrentQueue<T>::waitUntilEmpty() {
        for (;;) {
            {
                std::unique_lock lock(_lock);

                if (_size == 0) {
                    return;
                }
            }

            std::this_thread::yield();
        }
    }

    template <typename T>
    bool ConcurrentQueue<T>::isClosed() noexcept {
        std::unique_lock lock(_lock);
        return _closed;
    }

    template <typename T>
    template <typename InsertT>
    bool ConcurrentQueue<T>::tryEnque(InsertT&& value) {
        std::unique_lock lock(_lock);

        if (_closed) {
            return false;
        }

        if (_size - 1 == _mask) {
            return false;
        }

        new (&_buffer[(_start + _size) & _mask]) T(std::forward<InsertT>(value));
        ++_size;

        lock.unlock();
        _condition.notify_one();
        return true;
    }

    template <typename T>
    template <typename InsertT>
    bool ConcurrentQueue<T>::tryEnque(InsertT const& value) {
        std::unique_lock lock(_lock);

        if (_closed) {
            return false;
        }

        if (_size - 1 == _mask) {
            return false;
        }

        new (&_buffer[(_start + _size) & _mask]) T(value);
        ++_size;

        lock.unlock();
        _condition.notify_one();
        return true;
    }

    template <typename T>
    bool ConcurrentQueue<T>::tryDeque(T& out) {
        std::unique_lock lock(_lock);

        if (_size != 0) {
            out = reinterpret_cast<T&&>(_buffer[_start & _mask]);
            ++_start;
            --_size;
            return true;
        }

        return false;
    }

    template <typename T>
    template <typename InsertT>
    void ConcurrentQueue<T>::enqueWait(InsertT&& value) {
        while (!tryEnque(std::forward<InsertT>(value))) {
            std::this_thread::yield();
        }
    }

    template <typename T>
    template <typename InsertT>
    void ConcurrentQueue<T>::enqueWait(InsertT const& value) {
        while (!tryEnque(value)) {
            std::this_thread::yield();
        }
    }

    template <typename T>
    bool ConcurrentQueue<T>::dequeWait(T& out) {
        std::unique_lock lock(_lock);
        for (;;) {
            _condition.wait(lock, [this] { return _size != 0 || _closed; });

            // check for spurious wakeup
            if (_size != 0) {
                out = reinterpret_cast<T&&>(_buffer[_start & _mask]);
                ++_start;
                --_size;
                return true;
            }

            if (_closed) {
                return false;
            }
        }
    }

    template <typename T>
    void ConcurrentQueue<T>::close() {
        std::unique_lock lock(_lock);

        _closed = true;
        _condition.notify_all();
    }

} // namespace up
