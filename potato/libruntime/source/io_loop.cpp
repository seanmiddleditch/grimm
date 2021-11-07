// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/io_loop.h"

#include <cstring>
#include <uv.h>

namespace {
    struct StateVirtualBase {
        virtual ~StateVirtualBase() = default;
        uv_any_handle handle = {};
    };

    template <typename HandleT>
    struct StateBase : StateVirtualBase {
        HandleT handle = {};
    };

    template <typename StateT>
    void close(StateT*& state) {
        if (state != nullptr) {
            uv_close(reinterpret_cast<uv_handle_t*>(&state->handle), [](uv_handle_t* handle) {
                auto* state = static_cast<StateVirtualBase*>(handle->data);
                delete state; // NOLINT
            });
            state = nullptr;
        }
    }
} // namespace

struct up::IOEvent::State : StateBase<uv_async_t> {
    Callback callback;
};

up::IOEvent::IOEvent(uv_loop_t* loop, Callback callback) {
    UP_ASSERT(loop != nullptr);

    _state = new State; // NOLINT
    uv_async_init(loop, &_state->handle, [](uv_async_t* async) {
        auto* state = static_cast<State*>(async->data);
        if (state->callback) {
            state->callback();
        }
    });
    _state->handle.data = _state;
    _state->callback = std::move(callback);
}

void up::IOEvent::signal() {
    UP_GUARD_VOID(_state != nullptr);

    uv_async_send(&_state->handle);
}

void up::IOEvent::reset() {
    close(_state);
}

struct up::IOStream::State : StateBase<uv_any_handle> {
    ReadCallback readCallback;
    DisconnectCallback disconnectCallback;
};

struct up::IOStream::WriteReq {
    uv_write_t req;
    uv_buf_t buf;
};

up::IOStream::IOStream(uv_loop_t* loop) {
    UP_ASSERT(loop != nullptr);

    _state = new State; // NOLINT
    uv_pipe_init(loop, &_state->handle.pipe, 0);
    _state->handle.pipe.data = _state;
}

up::IOStream::IOStream(uv_loop_t* loop, int fd) {
    UP_ASSERT(loop != nullptr);

    _state = new State; // NOLINT
    uv_pipe_init(loop, &_state->handle.pipe, 0);
    _state->handle.pipe.data = _state;

    uv_pipe_open(&_state->handle.pipe, fd);
}

void up::IOStream::startRead(ReadCallback callback) {
    UP_GUARD_VOID(_state != nullptr);
    UP_GUARD_VOID(callback);

    _state->readCallback = move(callback);

    uv_read_start(
        &_state->handle.stream,
        [](uv_handle_t*, size_t suggestedSize, uv_buf_t* buf) {
            buf->base = (char*)malloc(suggestedSize); // NOLINT
            buf->len = static_cast<unsigned long>(suggestedSize);
        },
        [](uv_stream_t* stream, ssize_t readSize, const uv_buf_t* buf) {
            auto* state = static_cast<State*>(stream->data);

            if (readSize <= 0) {
                if (state->disconnectCallback) {
                    state->disconnectCallback();
                }
            }

            if (buf != nullptr) {
                if (readSize > 0) {
                    state->readCallback({buf->base, static_cast<size_t>(readSize)});
                }

                free(buf->base); // NOLINT
            }
        });
}

void up::IOStream::stopRead() {
    UP_GUARD_VOID(_state != nullptr);

    uv_read_stop(&_state->handle.stream);
}

void up::IOStream::onDisconnect(DisconnectCallback callback) {
    UP_GUARD_VOID(_state != nullptr);

    _state->disconnectCallback = move(callback);
}

void up::IOStream::write(view<char> bytes) {
    UP_GUARD_VOID(_state != nullptr);

    if (bytes.empty()) {
        return;
    }

    auto* req = new WriteReq; // NOLINT;
    req->buf = uv_buf_init((char*)malloc(bytes.size()), static_cast<unsigned int>(bytes.size())); // NOLINT
    std::memcpy(req->buf.base, bytes.data(), bytes.size());
    uv_write(&req->req, &_state->handle.stream, &req->buf, 1, [](uv_write_t* req, int status) {
        auto* writeReq = static_cast<WriteReq*>(req->data);
        free(writeReq->buf.base); // NOLINT
        delete writeReq; // NOLINT
    });
    req->req.data = req;
}

void up::IOStream::reset() {
    close(_state);
}

uv_stream_t* up::IOStream::rawStream() const noexcept {
    return _state != nullptr ? &_state->handle.stream : nullptr;
}

struct up::IOWatch::State : StateBase<uv_fs_event_t> {
    Callback callback;
};

up::IOWatch::IOWatch(uv_loop_t* loop, zstring_view targetFilename, Callback callback) {
    UP_ASSERT(loop != nullptr);
    UP_ASSERT(targetFilename);
    UP_ASSERT(callback);

    _state = new State; // NOLINT
    uv_fs_event_init(loop, &_state->handle);
    _state->handle.data = _state;
    _state->callback = std::move(callback);

    uv_fs_event_start(
        &_state->handle,
        [](uv_fs_event_t* ev, char const* filename, int events, int status) {
            if (filename == nullptr) {
                return;
            }

            auto* state = static_cast<State*>(ev->data);
            if ((events & UV_CHANGE) != 0) {
                state->callback(filename, IOWatchEvent::Change);
            }
            if ((events & UV_RENAME) != 0) {
                state->callback(filename, IOWatchEvent::Rename);
            }
        },
        targetFilename.c_str(),
        UV_FS_EVENT_RECURSIVE);
}

void up::IOWatch::reset() {
    close(_state);
}

struct up::IOPrepareHook::State : StateBase<uv_prepare_t> {
    Callback callback;
};

up::IOPrepareHook::IOPrepareHook(uv_loop_t* loop, Callback callback) {
    UP_ASSERT(loop != nullptr);
    UP_ASSERT(callback);

    _state = new State; // NOLINT
    uv_prepare_init(loop, &_state->handle);
    _state->handle.data = _state;
    _state->callback = std::move(callback);

    uv_prepare_start(&_state->handle, [](uv_prepare_t* prepare) {
        auto* state = static_cast<State*>(prepare->data);
        state->callback();
    });
}

void up::IOPrepareHook::reset() {
    close(_state);
}

struct up::IOLoop::State {
    uv_loop_t loop;
};

struct up::IOProcess::State : StateBase<uv_process_t> {
    uv_loop_t* loop = nullptr;
    bool initialized = false;
};

up::IOProcess::IOProcess(uv_loop_t* loop) {
    _state = new State; // NOLINT
    _state->loop = loop;
}

int up::IOProcess::spawn(IOProcessConfig const& config) {
    UP_ASSERT(_state != nullptr);
    UP_ASSERT(!_state->initialized);

    uv_stdio_container_t stdio[] = {{.flags = UV_IGNORE}, {.flags = UV_IGNORE}, {.flags = UV_IGNORE}};

    if (config.input != nullptr) {
        stdio[0] = {
            .flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_READABLE_PIPE),
            .data = {.stream = config.input->rawStream()}};
    }
    if (config.output != nullptr) {
        stdio[1] = {
            .flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE),
            .data = {.stream = config.output->rawStream()}};
    }

    uv_process_options_t options = {
        .file = "recon",
        .args = (char**)config.args.data(), // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        .flags = UV_PROCESS_WINDOWS_HIDE,
        .stdio_count = sizeof(stdio) / sizeof(stdio[0]),
        .stdio = stdio};

    _state->initialized = true;

    return uv_spawn(_state->loop, &_state->handle, &options);
}

int up::IOProcess::pid() const noexcept {
    UP_ASSERT(_state != nullptr);
    UP_ASSERT(_state->initialized);

    return uv_process_get_pid(&_state->handle);
}

void up::IOProcess::terminate(bool kill) {
    UP_ASSERT(_state != nullptr);
    UP_ASSERT(_state->initialized);

    uv_process_kill(&_state->handle, kill ? SIGKILL : SIGTERM);
}

bool up::IOProcess::empty() const noexcept {
    return _state == nullptr || !_state->initialized;
}

void up::IOProcess::reset() {
    if (_state != nullptr) {
        if (_state->initialized) {
            close(_state);
        }
        else {
            delete _state;
            _state = nullptr;
        }
    }
}

up::IOLoop::IOLoop() {
    _state = new State; // NOLINT
    uv_loop_init(&_state->loop);
    _state->loop.data = _state;
}

up::IOEvent up::IOLoop::createEvent(delegate<void()> callback) {
    UP_ASSERT(_state != nullptr);
    return IOEvent(&_state->loop, std::move(callback));
}

up::IOStream up::IOLoop::createPipe() {
    UP_ASSERT(_state != nullptr);
    return IOStream(&_state->loop);
}

up::IOStream up::IOLoop::createPipeFor(int fd) {
    UP_ASSERT(_state != nullptr);
    UP_ASSERT(fd >= 0);
    return IOStream(&_state->loop, fd);
}

up::IOWatch up::IOLoop::createWatch(zstring_view targetFilename, IOWatch::Callback callback) {
    UP_ASSERT(_state != nullptr);
    return IOWatch(&_state->loop, targetFilename, std::move(callback));
}

up::IOPrepareHook up::IOLoop::createPrepareHook(IOPrepareHook::Callback callback) {
    UP_ASSERT(_state != nullptr);
    return IOPrepareHook(&_state->loop, std::move(callback));
}

up::IOProcess up::IOLoop::createProcess() {
    UP_ASSERT(_state != nullptr);
    return IOProcess(&_state->loop);
}

bool up::IOLoop::run(IORun run) {
    UP_GUARD(_state != nullptr, false);
    return uv_run(
               &_state->loop,
               run == IORun::Poll          ? UV_RUN_NOWAIT
                   : run == IORun::WaitOne ? UV_RUN_ONCE
                                           : UV_RUN_DEFAULT) != 0;
}

void up::IOLoop::stop() noexcept {
    UP_GUARD_VOID(_state != nullptr);
    uv_stop(&_state->loop);
}

up::zstring_view up::IOLoop::errorString(int code) noexcept {
    return uv_strerror(code);
}

void up::IOLoop::reset() {
    if (_state != nullptr) {
        uv_run(&_state->loop, UV_RUN_DEFAULT);

        [[maybe_unused]] int const rc = uv_loop_close(&_state->loop);
        UP_ASSERT(rc == 0, "error: {}: {}", rc, uv_strerror(rc));

        delete _state; // NOLINT
        _state = nullptr;
    }
}
