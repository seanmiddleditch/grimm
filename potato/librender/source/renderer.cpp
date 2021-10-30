// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/renderer.h"

#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_renderable.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"

#include <chrono>

constexpr int debug_vbo_size = 64 * 1024;
constexpr double nano_to_seconds = 1.0 / 1000000000.0;

up::Renderer::Renderer(rc<GpuDevice> device) : _device(std::move(device)) {
    //_debugLineMaterial = _loader.loadMaterialSync("materials/debug_line.mat");
    //_debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, debug_vbo_size);

    // Create the debug pipeline
    GpuPipelineStateDesc pipelineDesc;

    GpuInputLayoutElement const layout[] = {
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
        {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
    };

    pipelineDesc.enableDepthTest = true;
    pipelineDesc.enableDepthWrite = true;
    pipelineDesc.vertShader = _device->getDebugShader(GpuShaderStage::Vertex).as_bytes();
    pipelineDesc.pixelShader = _device->getDebugShader(GpuShaderStage::Pixel).as_bytes();
    pipelineDesc.inputLayout = layout;
    pipelineDesc.signatureType = RootSignatureType::eRST_DebugDraw;

    // Check to support null renderer; should this be explicit?
    if (!pipelineDesc.vertShader.empty() && !pipelineDesc.pixelShader.empty()) {
        _debugState = _device->createPipelineState(pipelineDesc);
    }
}

up::Renderer::~Renderer() = default;

void up::Renderer::flush() {
    constexpr double nanoToSeconds = 1.0 / 1000000000.0;
    UP_ASSERT(_swapChain.get() != nullptr);
    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(GpuBufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    auto const now = static_cast<double>(nowNanoseconds - _startTimestamp) * nanoToSeconds;
    auto const delta = static_cast<float>(now - _frameTimestamp);

    FrameData frame = {_frameCounter++, delta, now};
    _frameTimestamp = now;

    _device->beginFrame(_swapChain.get());

    _device->debugDraw([&](GpuCommandList& cmdList) { flushDebugDraw(cmdList, delta); });

    for (auto& renderable : _rendarables) {
        _device->render(frame, renderable.get());
    }
    _rendarables.clear();
    _device->endFrame(_swapChain.get());

    _device->execute(false);
    _swapChain->present();
}

void up::Renderer::quit() {
    _device->execute(true);
}

auto up::Renderer::createRendarable(IRenderable* pInterface) -> GpuRenderable* {
    auto renderable = _device->createRenderable(pInterface);
    return _rendarables.emplace_back(std::move(renderable)).get();
}

void up::Renderer::flushDebugDraw(GpuCommandList& cmdList, float frameTime) {
    static constexpr uint32 bufferSize = 64 * 1024;
    static constexpr uint32 maxVertsPerChunk = bufferSize / sizeof(DebugDrawVertex);

    if (_debugState.empty()) {
        up::flushDebugDraw(frameTime);
        return;
    }

    if (_debugBuffer == nullptr) {
        _debugBuffer = _device->createBuffer(GpuBufferType::Vertex, bufferSize);
    }

    cmdList.setPipelineState(_debugState.get());
    cmdList.bindVertexBuffer(0, _debugBuffer.get(), sizeof(DebugDrawVertex));
    cmdList.setPrimitiveTopology(GpuPrimitiveTopology::Lines);

    dumpDebugDraw([&](auto debugVertices) {
        if (debugVertices.empty()) {
            return;
        }

        uint32 vertCount = min(static_cast<uint32>(debugVertices.size()), maxVertsPerChunk);
        uint32 offset = 0;
        while (offset < debugVertices.size()) {
            cmdList.update(_debugBuffer.get(), debugVertices.subspan(offset, vertCount).as_bytes());
            cmdList.draw(vertCount);

            offset += vertCount;
            vertCount = min(static_cast<uint32>(debugVertices.size()) - offset, maxVertsPerChunk);
        }
    });

    up::flushDebugDraw(frameTime);
}

namespace up {
    namespace {
        class MeshAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Mesh::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Mesh::createFromBuffer(ctx.key, contents);
            }
        };

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

        class TextureAssetLoaderBackend : public AssetLoaderBackend {
        public:
            TextureAssetLoaderBackend(Renderer& renderer) : _renderer(renderer) { }

            zstring_view typeName() const noexcept override { return Texture::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                auto img = loadImage(ctx.stream);
                ctx.stream.close();
                if (img.data().empty()) {
                    return nullptr;
                }

                GpuTextureDesc desc = {};
                desc.type = GpuTextureType::Texture2D;
                desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
                desc.width = img.header().width;
                desc.height = img.header().height;

                auto tex = _renderer.device().createTexture2D(desc, img.data());
                if (tex == nullptr) {
                    return nullptr;
                }

                return new_shared<Texture>(ctx.key, std::move(img), std::move(tex));
            }

        private:
            Renderer& _renderer;
        };
    } // namespace
} // namespace up

void up::Renderer::registerAssetBackends(AssetLoader& assetLoader) {
    UP_ASSERT(_device != nullptr);
    assetLoader.registerBackend(new_box<MeshAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<MaterialAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<ShaderAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<TextureAssetLoaderBackend>(*this));
    _device->registerAssetBackends(assetLoader);
}

bool up::Renderer::createSwapChain(void* nativeWindow) {
    UP_ASSERT(_device != nullptr);
    _swapChain = _device->createSwapChain(nativeWindow);
    if (!_swapChain)
        return false;
    return true;
}

void up::Renderer::resizeBuffers(int width, int height) {
    UP_ASSERT(_device != nullptr);
    _swapChain->resizeBuffers(*_device.get(), width, height);
}

auto up::Renderer::getBackBuffer() -> rc<GpuTexture> {
    return _swapChain->getBuffer(_swapChain->getCurrentBufferIndex());
}

void up::Renderer::clearCommandList() {
    _device->clearCommandList();
}
