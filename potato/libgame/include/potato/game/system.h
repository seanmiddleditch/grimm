// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    class RenderContext;
    class Space;

    class System {
    public:
        explicit System(Space& space) noexcept : m_space(space) {}
        virtual ~System() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void update(float deltaTime) = 0;
        virtual void render(RenderContext&) {}

        protected:
        Space& space() noexcept { return m_space; }

    private:
        Space& m_space;
    };
} // namespace up
