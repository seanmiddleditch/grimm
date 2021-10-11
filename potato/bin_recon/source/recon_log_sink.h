// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/logger.h"

namespace up::recon {
    class ReconServer;

    class ReconProtocolLogSink : public LogSink {
    public:
        explicit ReconProtocolLogSink(ReconServer& server) : _server(server) {}
        ~ReconProtocolLogSink() override = default;

        void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept
            override;

    private:
        ReconServer& _server;
    };
} // namespace up::recon
