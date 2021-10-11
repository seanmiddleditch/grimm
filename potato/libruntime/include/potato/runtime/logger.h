// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "lock_guard.h"
#include "rwlock.h"

#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <mutex>
#include <utility>

namespace up {
    enum class LogSeverity { Info, Error };

    namespace log_severity_mask {
        enum class LogSeverityMask : unsigned {
            Info = 1 << (int)LogSeverity::Info,
            Error = 1 << (int)LogSeverity::Error,
            Everything = Info | Error
        };

        constexpr LogSeverityMask operator~(LogSeverityMask mask) noexcept {
            return LogSeverityMask{~static_cast<unsigned>(mask)};
        }

        constexpr LogSeverityMask operator|(LogSeverityMask a, LogSeverityMask b) noexcept {
            return LogSeverityMask{static_cast<unsigned>(a) | static_cast<unsigned>(b)};
        }

        constexpr LogSeverityMask operator&(LogSeverityMask a, LogSeverityMask b) noexcept {
            return LogSeverityMask{static_cast<unsigned>(a) & static_cast<unsigned>(b)};
        }
    } // namespace log_severity_mask

    using LogSeverityMask = log_severity_mask::LogSeverityMask;

    constexpr LogSeverityMask toMask(LogSeverity severity) noexcept {
        return static_cast<LogSeverityMask>(1 << static_cast<int>(severity));
    }

    constexpr LogSeverityMask toInclusiveMask(LogSeverity severity) noexcept {
        unsigned const high = 1 << static_cast<int>(severity);
        unsigned const rest = high - 1;
        return static_cast<LogSeverityMask>(high | rest);
    }

    constexpr zstring_view toString(LogSeverity severity) noexcept {
        switch (severity) {
            case LogSeverity::Info:
                return "info"_zsv;
            case LogSeverity::Error:
                return "error"_zsv;
            default:
                return "unknown"_zsv;
        }
    }

    struct LogLocation {
        zstring_view file;
        zstring_view function;
        int line = 0;
    };

    class LogSink : public shared<LogSink> {
    public:
        virtual ~LogSink() = default;

        virtual void log(
            string_view loggerName,
            LogSeverity severity,
            string_view message,
            LogLocation location = {}) noexcept = 0;

    protected:
        UP_RUNTIME_API void next(
            string_view loggerName,
            LogSeverity severity,
            string_view message,
            LogLocation location) noexcept;

    private:
        rc<LogSink> _next;

        friend class Logger;
    };

    class DefaultLogSink final : public LogSink {
        void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept
            override;
    };

    class Logger {
    public:
        Logger(string name, LogSeverity minimumSeverity = LogSeverity::Info)
            : Logger(std::move(name), root()._impl, nullptr, minimumSeverity) {}
        Logger(string name, Logger& parent, LogSeverity minimumSeverity = LogSeverity::Info)
            : Logger(std::move(name), parent._impl, nullptr, minimumSeverity) {}
        Logger(string name, Logger& parent, rc<LogSink> sink, LogSeverity minimumSeverity = LogSeverity::Info)
            : Logger(std::move(name), parent._impl, std::move(sink), minimumSeverity) {}

        UP_RUNTIME_API ~Logger();

        Logger(Logger const&) = delete;
        Logger& operator=(Logger const&) = delete;

        UP_RUNTIME_API static Logger& root();

        UP_RUNTIME_API bool isEnabledFor(LogSeverity severity) const noexcept;

        template <typename... T>
        void log(LogSeverity severity, nanofmt::format_string format, T const&... args);
        UP_RUNTIME_API void log(LogSeverity severity, string_view message) noexcept;

        template <typename... T>
        void info(nanofmt::format_string format, T const&... args) {
            log(LogSeverity::Info, format, args...);
        }
        void info(string_view message) noexcept { log(LogSeverity::Info, message); }

        template <typename... T>
        void error(nanofmt::format_string format, T const&... args) {
            log(LogSeverity::Error, format, args...);
        }
        void error(string_view message) noexcept { log(LogSeverity::Error, message); }

        UP_RUNTIME_API void attach(rc<LogSink> sink) noexcept;
        UP_RUNTIME_API void detach(LogSink* sink) noexcept;

    private:
        static constexpr int log_length = 1024;

        struct Impl : up::shared<Impl> {
            string name;
            LogSeverity minimumSeverity = LogSeverity::Info;
            RWLock lock;
            rc<LogSink> sink;
            rc<Impl> parent;
        };

        UP_RUNTIME_API Logger(string name, rc<Impl> parent, rc<LogSink> sink, LogSeverity minimumSeverity);
        static void _dispatch(Impl& impl, LogSeverity severity, string_view loggerName, string_view message) noexcept;

        rc<Impl> _impl;
    };

    template <typename... T>
    void Logger::log(LogSeverity severity, nanofmt::format_string format, T const&... args) {
        if (!isEnabledFor(severity)) {
            return;
        }

        char buffer[log_length] = {};
        nanofmt::format_to(buffer, format, args...);

        log(severity, buffer);
    }
} // namespace up
