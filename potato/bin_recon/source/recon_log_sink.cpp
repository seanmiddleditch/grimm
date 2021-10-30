// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_log_sink.h"

#include "potato/recon/recon_protocol.h"
#include "potato/recon/recon_server.h"
#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>
#include <iostream>

void up::recon::ReconProtocolLogSink::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    _server.send<ReconLogMessage>({.category = string(loggerName), .message = string(message), .severity = severity});
}
