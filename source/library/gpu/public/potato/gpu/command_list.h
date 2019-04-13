// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/foundation/span.h"
#include "potato/foundation/int_types.h"
#include <glm/vec4.hpp>

namespace up::gpu {
    class GpuBuffer;
    class ResourceView;
    class PipelineState;
    class Sampler;
    class Texture;

    class GpuCommandList {
    public:
        GpuCommandList() = default;
        virtual ~GpuCommandList() = default;

        GpuCommandList(GpuCommandList&&) = delete;
        GpuCommandList& operator=(GpuCommandList&&) = delete;

        virtual void setPipelineState(PipelineState* state) = 0;

        virtual void bindRenderTarget(uint32 index, ResourceView* view) = 0;
        virtual void bindDepthStencil(ResourceView* view) = 0;
        virtual void bindIndexBuffer(GpuBuffer* buffer, IndexType indexType, uint32 offset = 0) = 0;
        virtual void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) = 0;
        virtual void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, ShaderStage stage) = 0;
        virtual void bindShaderResource(uint32 slot, ResourceView* view, ShaderStage stage) = 0;
        virtual void bindSampler(uint32 slot, Sampler* sampler, ShaderStage stage) = 0;
        virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
        virtual void setViewport(Viewport const& viewport) = 0;
        virtual void setClipRect(Rect rect) = 0;

        virtual void draw(uint32 vertexCount, uint32 firstVertex = 0) = 0;
        virtual void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) = 0;

        virtual void clearRenderTarget(ResourceView* view, glm::vec4 color) = 0;
        virtual void clearDepthStencil(ResourceView* view) = 0;

        virtual void finish() = 0;
        virtual void clear(PipelineState* pipelineState = nullptr) = 0;

        virtual span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) = 0;
        virtual void unmap(GpuBuffer* resource, span<byte const> data) = 0;
        virtual void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) = 0;
    };
} // namespace up::gpu
