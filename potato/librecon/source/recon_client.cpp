// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>

static up::Logger s_logger("ReconClient"); // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

up::ReconClient::ReconClient() {
    on<ReconLogMessage>([](schema::ReconLogMessage const& msg) { s_logger.log(msg.severity, msg.message); });
}

bool up::ReconClient::start(IOLoop& loop, zstring_view projectPath) {
    UP_ASSERT(!projectPath.empty());

    const char* const args[] = {"recon", "-project", projectPath.c_str(), "-server", nullptr};

    _sink = loop.createPipe();
    _source = loop.createPipe();

    IOProcessConfig config{
        .process = "recon",
        .args = args,
        .input = &_sink,
        .output = &_source,
    };

    _process = loop.createProcess();
    auto const ec = _process.spawn(config);

    if (ec != 0) {
        s_logger.error("Failed to start recon: {}", IOLoop::errorString(ec));
        return false;
    }

    _source.startRead([this](auto input) { receive(input); });

    s_logger.info("Started recon PID={}", _process.pid());

    return true;
}

void up::ReconClient::stop() {
    _source.reset();
    _sink.reset();

    if (_process) {
        _process.terminate(true);
        _process.reset();
    }
}
