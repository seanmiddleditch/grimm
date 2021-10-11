// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_protocol.h"

#include "potato/runtime/io_loop.h"

namespace up {
    class Project;

    class ReconClient : public ReconProtocol {
    public:
        UP_RECON_API ReconClient();
        ~ReconClient() { stop(); }

        bool UP_RECON_API start(IOLoop& loop, zstring_view projectPath);
        void UP_RECON_API stop();

    private:
        IOStream& sink() override { return _sink; }

        IOProcess _process;
        IOStream _sink;
        IOStream _source;
    };
} // namespace up
