// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/renderer.h"

#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_resource.h"
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
        _frameDataBuffer = _device->createBuffer(GpuBufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
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

    return commandList;
}

namespace up {
    namespace {
        class MaterialAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Material::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Material::createFromBuffer(ctx.key, contents, ctx.loader);
            }
        };

        class ShaderAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Shader::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return up::new_shared<Shader>(ctx.key, std::move(contents));
            }
        };
    } // namespace
} // namespace up

void up::Renderer::registerAssetBackends(AssetLoader& assetLoader) {
    UP_ASSERT(_device != nullptr);
    assetLoader.registerBackend(new_box<MaterialAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<ShaderAssetLoaderBackend>());
}

void up::Renderer::renderDebugDraw(GpuCommandList& commandList) {
    _device->renderDebugDraw(commandList);
    flushDebugDraw(static_cast<float>(_frameTimestep));
}
