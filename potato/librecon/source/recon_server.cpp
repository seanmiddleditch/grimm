// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_server.h"

#include <nlohmann/json.hpp>
#include <iostream>

up::recon::ReconServer::ReconServer(Logger& logger) : _logger("ReconServer", logger) {}

void up::recon::ReconServer::onDisconnect(DisconnectHandler handler) {
    _disconnectHandler = std::move(handler);
}

bool up::recon::ReconServer::start(IOLoop& loop) {
    _source = loop.createPipeFor(0);
    _sink = loop.createPipeFor(1);

    _source.startRead([this](span<char> input) {
        if (!receive(input)) {
            _logger.error("Unhandled JSON message");
        }
    });

    _source.onDisconnect([this] {
        if (_disconnectHandler) {
            _disconnectHandler();
        }
    });
    return true;
}

void up::recon::ReconServer::stop() {
    _sink.reset();
    _source.reset();
}
