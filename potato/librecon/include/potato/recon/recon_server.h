// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_protocol.h"

#include "potato/runtime/io_loop.h"
#include "potato/runtime/logger.h"
#include "potato/spud/delegate.h"

namespace up::recon {
    class ReconServer : public ReconProtocol {
    public:
        using DisconnectHandler = delegate<void()>;

        UP_RECON_API explicit ReconServer(Logger& logger);
        ~ReconServer() { stop(); }

        UP_RECON_API void onDisconnect(DisconnectHandler handler);

        UP_RECON_API bool start(IOLoop& loop);
        UP_RECON_API void stop();

    private:
        IOStream& sink() override { return _sink; }

        Logger _logger;
        IOStream _sink;
        IOStream _source;
        DisconnectHandler _disconnectHandler;
    };
} // namespace up::recon
