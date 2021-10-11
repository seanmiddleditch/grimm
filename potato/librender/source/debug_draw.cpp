// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "debug_draw.h"
#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_pipeline_state.h"

#include "potato/spud/vector.h"

#include <mutex>

static std::mutex debugLock; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static up::vector<up::DebugDrawVertex> debugVertices; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void UP_VECTORCALL
drawDebugLineHelper(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds = 0) {
    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({end, color, lingerSeconds});
}

static void UP_VECTORCALL
drawDebugRayHelper(glm::vec3 start, glm::vec3 direction, float length, glm::vec4 color, float lingerSeconds = 0) {
    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({start + direction * length, color, lingerSeconds});
}

void UP_VECTORCALL up::drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds) {
    std::unique_lock _(debugLock);
    drawDebugLineHelper(start, end, color, lingerSeconds);
}

void UP_VECTORCALL up::drawDebugGrid(DebugDrawGrid const& grid) {
    std::unique_lock _(debugLock);

    auto const start1 = grid.offset - grid.axis1 * static_cast<float>(grid.halfWidth);
    auto const start2 = grid.offset - grid.axis2 * static_cast<float>(grid.halfWidth);
    auto const width = static_cast<float>(grid.halfWidth + grid.halfWidth);

    for (int i = 0; i <= grid.halfWidth; i += grid.spacing) {
        auto const color = i % grid.guidelineSpacing != 0 ? grid.lineColor : grid.guidelineColor;
        auto const f = static_cast<float>(i);
        drawDebugRayHelper(start1 + grid.axis2 * f, grid.axis1, width, color);
        drawDebugRayHelper(start1 - grid.axis2 * f, grid.axis1, width, color);
        drawDebugRayHelper(start2 + grid.axis1 * f, grid.axis2, width, color);
        drawDebugRayHelper(start2 - grid.axis1 * f, grid.axis2, width, color);
    }
}

void up::dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback) {
    std::unique_lock _(debugLock);
    callback(span{debugVertices.data(), debugVertices.size()});
}

void up::flushDebugDraw(float frameTime) {
    std::unique_lock _(debugLock);

    decltype(debugVertices.size()) outCount = 0;
    for (auto const& vert : debugVertices) {
        if (vert.linger >= frameTime) {
            debugVertices[outCount] = vert;
            debugVertices[outCount++].linger -= frameTime;
        }
    }

    debugVertices.resize(outCount);
}
