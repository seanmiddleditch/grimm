// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "concurrent_queue.h"
#include "thread_util.h"

#include "potato/spud/delegate.h"
#include "potato/spud/zstring_view.h"

#include <thread>

namespace up {
    using Task = delegate<void()>;
    using TaskQueue = ConcurrentQueue<Task>;

    class TaskWorker {
    public:
        UP_RUNTIME_API explicit TaskWorker(TaskQueue& queue, zstring_view name);
        UP_RUNTIME_API ~TaskWorker();

        TaskWorker(TaskWorker&&) = delete;
        TaskWorker& operator=(TaskWorker&&) = delete;

        SmallThreadId smallThreadId() const noexcept { return _threadId; }

        void detach() { _thread.detach(); }
        void join() { _thread.join(); }

    private:
        int _threadMain();

        TaskQueue& _queue;
        SmallThreadId _threadId;
        std::thread _thread;
    };

} // namespace up
