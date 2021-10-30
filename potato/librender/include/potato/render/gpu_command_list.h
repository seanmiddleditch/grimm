// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/int_types.h"
#include "potato/spud/span.h"

#include <glm/vec4.hpp>

namespace up {
    class GpuBuffer;
    class GpuResourceView;
    class GpuPipelineState;
    class GpuSampler;
    class GpuTexture;

    class GpuCommandList {
    public:
        GpuCommandList() = default;
        virtual ~GpuCommandList() = default;

        GpuCommandList(GpuCommandList&&) = delete;
        GpuCommandList& operator=(GpuCommandList&&) = delete;

        virtual void setPipelineState(GpuPipelineState* state) = 0;

        virtual void bindRenderTarget(uint32 index, GpuResourceView* view) = 0;
        virtual void bindDepthStencil(GpuResourceView* view) = 0;
        virtual void bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, uint32 offset = 0) = 0;
        virtual void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) = 0;
        virtual void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) = 0;
        virtual void bindConstantValues(uint32 count, float* values, GpuShaderStage stage) = 0;
        virtual void bindShaderResource(uint32 slot, GpuResourceView* view, GpuShaderStage stage) = 0;
        virtual void bindTexture(uint32 slot, GpuResourceView* view, GpuSampler* sampler, GpuShaderStage stage) = 0;
        virtual void setPrimitiveTopology(GpuPrimitiveTopology topology) = 0;
        virtual void setViewport(GpuViewportDesc const& viewport) = 0;
        virtual void setClipRect(GpuClipRect rect) = 0;

        virtual void draw(uint32 vertexCount, uint32 firstVertex = 0) = 0;
        virtual void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) = 0;

        virtual void clearRenderTarget(GpuResourceView* view, glm::vec4 color) = 0;
        virtual void clearDepthStencil(GpuResourceView* view) = 0;

        virtual void start(GpuPipelineState* pipelineState) = 0;
        virtual void finish() = 0;
        virtual void clear(GpuPipelineState* pipelineState = nullptr) = 0;

        virtual span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) = 0;
        virtual void unmap(GpuBuffer* resource, span<byte const> data) = 0;
        virtual void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) = 0;
    };
} // namespace up
