// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/renderer.h"

#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_sampler.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/numeric_util.h"

#include <chrono>

namespace {
    struct FrameData {
        up::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

up::Renderer::Renderer(rc<GpuDevice> device) : _device(std::move(device)) { }

up::Renderer::~Renderer() = default;

void up::Renderer::beginFrame() {
    constexpr double nanoToSeconds = 1.0 / 1000000000.0;

    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer({.type = GpuBufferType::Constant, .size = sizeof(FrameData)}, {});
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    if (_linearSampler == nullptr) {
        _linearSampler =
            _device->createSampler({.address = GpuTextureAddressMode::Clamp, .filter = GpuFilter::MinMagMip_Point});
    }
    if (_bilinearSampler == nullptr) {
        _bilinearSampler = _device->createSampler(
            {.address = GpuTextureAddressMode::Clamp, .filter = GpuFilter::MinMag_Linear_Mip_Point});
    }

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) * nanoToSeconds;
    _frameTimestep = static_cast<float>(now - _frameTimestamp);
    _frameTimestamp = now;

    ++_frameCounter;
}

auto up::Renderer::context() -> RenderContext {
    return RenderContext{*_device, createCommandList()};
}

auto up::Renderer::createCommandList() const noexcept -> rc<GpuCommandList> {
    rc<GpuCommandList> commandList = _device->createCommandList();

    FrameData frame = {_frameCounter, static_cast<float>(_frameTimestep), _frameTimestamp};

    commandList->begin();
    commandList->update(_frameDataBuffer.get(), view<byte>{reinterpret_cast<byte*>(&frame), sizeof(frame)});
    commandList->bindConstantBuffer(0, _frameDataBuffer.get(), GpuShaderStage::All);
    commandList->bindSampler(0, _linearSampler.get(), GpuShaderStage::All);
    commandList->bindSampler(1, _bilinearSampler.get(), GpuShaderStage::All);

    return commandList;
}

void up::Renderer::renderDebugDraw(GpuCommandList& commandList) {
    _device->renderDebugDraw(commandList);
    flushDebugDraw(static_cast<float>(_frameTimestep));
}
