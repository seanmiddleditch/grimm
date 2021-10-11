// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "log_history.h"

class up::shell::LogHistory::LogHistorySink : public LogSink {
public:
    LogHistorySink(LogHistory& history) : _history(history) {}

    void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location) noexcept
        override {
        if (!_history._logs.empty()) {
            LogEntry& last = _history._logs.back();
            if (last.severity == severity && last.message == message) {
                ++last.count;
                return;
            }
        }

        _history._logs.push_back({severity, string(message), string(loggerName), string{}});

        next(loggerName, severity, message, location);
    }

    LogHistory& _history;
};

up::shell::LogHistory::LogHistory() : _sink(new_shared<LogHistorySink>(*this)) {
    Logger::root().attach(_sink);
}

up::shell::LogHistory::~LogHistory() {
    Logger::root().detach(_sink.get());
}
