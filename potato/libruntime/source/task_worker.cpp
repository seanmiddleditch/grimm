// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "task_worker.h"
#include "thread_util.h"

#include "potato/spud/string.h"

up::TaskWorker::TaskWorker(ConcurrentQueue<Task>& queue, zstring_view name) : _queue(queue) {
    // just to make sure this is called at least once on the main thread...
    [[maybe_unused]] auto const _ = currentSmallThreadId();

    _thread = std::thread([this, name = up::string(name)] {
        setCurrentThreadName(name);
        _threadId = currentSmallThreadId();
        return _threadMain();
    });
}

up::TaskWorker::~TaskWorker() {
    if (_thread.joinable()) {
        _thread.join();
    }
}

int up::TaskWorker::_threadMain() {
    Task task;
    while (_queue.dequeWait(task)) {
        task();
    }
    return 0;
}
