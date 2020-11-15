// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <reproc++/reproc.hpp>
#include <thread>

namespace up {
    class Project;
}

namespace up::shell {
    class ReconClient {
    public:
        ~ReconClient() { stop(); }

        bool start(Project& project);
        void stop();

    private:
        reproc::process _process;
        std::thread _thread;
    };
} // namespace up::shell
