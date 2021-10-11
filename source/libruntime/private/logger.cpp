// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "logger.h"

#if UP_PLATFORM_WINDOWS
#    include "potato/spud/platform_windows.h"
#endif

#include <nanofmt/format.h>
#include <iostream>

void up::DefaultLogSink::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    char buffer[2048] = {
        0,
    };
    char* end = buffer;

    if (location.file) {
        end = nanofmt::format_to_n(
            buffer,
            sizeof buffer - (end - buffer),
            "{}({}): <{}> ",
            location.file,
            location.line,
            location.function);
    }

    end = nanofmt::format_to_n(
        buffer,
        sizeof buffer - (end - buffer),
        "[{}] {} :: {}\n",
        toString(severity),
        loggerName,
        message);

    if (end == buffer + sizeof buffer) {
        --end;
    }
    *end = '\0';

    {
        std::ostream& os = severity == LogSeverity::Error ? std::cerr : std::cout;

        os.write(buffer, end - buffer);

        if (severity != LogSeverity::Info) {
            os.flush();
        }
    }

#if defined(UP_PLATFORM_WINDOWS)
    if (end == buffer + sizeof buffer)
        --end;
    *end = '\0';

    OutputDebugStringA(buffer);
#endif
}

void up::LogSink::next(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    if (_next != nullptr) {
        _next->log(loggerName, severity, message, location);
    }
}

up::Logger::Logger(string name, rc<Impl> parent, rc<LogSink> sink, LogSeverity minimumSeverity)
    : _impl(new_shared<Impl>()) {
    _impl->name = std::move(name);
    _impl->parent = std::move(parent);
    _impl->minimumSeverity = minimumSeverity;
    _impl->sink = std::move(sink);
}

up::Logger::~Logger() = default;

auto up::Logger::root() -> up::Logger& {
    static Logger s_logger("Potato", nullptr, new_shared<DefaultLogSink>(), LogSeverity::Info);
    return s_logger;
}

bool up::Logger::isEnabledFor(LogSeverity severity) const noexcept {
    return severity >= _impl->minimumSeverity;
}

void up::Logger::attach(rc<LogSink> sink) noexcept {
    LockGuard _(_impl->lock.writer());
    if (sink != nullptr) {
        sink->_next = std::move(_impl->sink);
    }
    _impl->sink = std::move(sink);
}

void up::Logger::detach(LogSink* sink) noexcept {
    if (sink == nullptr) {
        return;
    }

    LockGuard _(_impl->lock.writer());
    if (_impl->sink.get() == sink) {
        _impl->sink = std::move(_impl->sink->_next);
    }
}

void up::Logger::_dispatch(Impl& impl, LogSeverity severity, string_view loggerName, string_view message) noexcept {
    rc<Impl> parent;

    {
        LockGuard _(impl.lock.reader());
        if (impl.sink != nullptr) {
            impl.sink->log(loggerName, severity, message, {});
        }
        parent = impl.parent;
    }

    if (parent != nullptr && severity >= parent->minimumSeverity) {
        _dispatch(*parent, severity, loggerName, message);
    }
}

void up::Logger::log(LogSeverity severity, string_view message) noexcept {
    if (!isEnabledFor(severity)) {
        return;
    }

    rc<Impl> impl = _impl;

    _dispatch(*impl, severity, impl->name, message);
}
