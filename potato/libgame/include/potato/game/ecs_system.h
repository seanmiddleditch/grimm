// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    class System {
    public:
        virtual ~System();

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void update(float deltaTime) = 0;
    };
} // namespace up
