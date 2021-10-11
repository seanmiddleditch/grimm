// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/platform.h"
#include "potato/spud/span.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace up {

    struct DebugDrawVertex {
        glm::vec3 position;
        glm::vec4 color;
        float linger;
    };

    struct DebugDrawGrid {
        glm::vec4 lineColor = {0.3f, 0.3f, 0.3f, 1.f};
        glm::vec4 guidelineColor = {0.6f, 0.6f, 0.5f, 1.f};
        glm::vec3 offset = {0.f, 0.f, 0.f};
        glm::vec3 axis1 = {1.f, 0.f, 0.f};
        glm::vec3 axis2 = {0.f, 1.f, 0.f};
        int halfWidth = 100;
        int spacing = 1;
        int guidelineSpacing = 10;
    };

    UP_RENDER_API void UP_VECTORCALL
    drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = {1, 0, 0, 1}, float lingerSeconds = 0.f);
    UP_RENDER_API void UP_VECTORCALL drawDebugGrid(DebugDrawGrid const& grid);

    UP_RENDER_API void dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback);
    UP_RENDER_API void flushDebugDraw(float frameTime);

} // namespace up
