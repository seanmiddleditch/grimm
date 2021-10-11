// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/uuid.h"
#include "potato/spud/string.h"

#include <condition_variable>
#include <deque>
#include <mutex>

namespace up::recon {
    class ReconQueue {
    public:
        enum class Type { ImportAll, ForceImportAll, Import, ForceImport, Forget, Delete, Update, Terminate };
        struct Command {
            Type type = Type::Terminate;
            string filename;
            UUID uuid;
        };

        void enqueImport(string filename, bool force = false) {
            _enque({.type = force ? Type::ForceImport : Type::Import, .filename = std::move(filename)});
        }
        void enqueImport(UUID const& uuid, bool force = false) {
            _enque({.type = force ? Type::ForceImport : Type::Import, .uuid = uuid});
        }

        void enqueImportAll(bool force = false) { _enque({.type = force ? Type::ForceImportAll : Type::ImportAll}); }

        void enqueForget(string filename) { _enque({.type = Type::Forget, .filename = std::move(filename)}); }
        void enqueForget(UUID const& uuid) { _enque({.type = Type::Forget, .uuid = uuid}); }

        void enqueUpdate(string filename) { _enque({.type = Type::Update, .filename = std::move(filename)}); }
        void enqueUpdate(UUID const& uuid) { _enque({.type = Type::Update, .uuid = uuid}); }

        void enqueTerminate() { _enque({.type = Type::Terminate}); }

        inline void wait();
        inline bool tryDeque(Command& command);

    private:
        inline void _enque(Command&& command);

        std::mutex _mutex;
        std::condition_variable _signal;
        std::deque<Command> _queue;
    };

    void ReconQueue::wait() {
        std::unique_lock lock(_mutex);
        _signal.wait(lock, [this] { return !_queue.empty(); });
    }

    bool ReconQueue::tryDeque(Command& command) {
        std::unique_lock lock(_mutex);
        if (_queue.empty()) {
            return false;
        }

        command = std::move(_queue.front());
        _queue.pop_front();
        return true;
    }

    void ReconQueue::_enque(Command&& command) {
        std::unique_lock lock(_mutex);
        _queue.push_back(std::move(command));
        _signal.notify_all();
    }

} // namespace up::recon
