// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/renderer.h"
#include "potato/render/render_task.h"
#include "potato/render/context.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/render/debug_draw.h"
#include "potato/gpu/buffer.h"
#include "potato/gpu/command_list.h"
#include "potato/gpu/device.h"
#include "potato/gpu/swap_chain.h"
#include "potato/gpu/texture.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/stream_util.h"
#include "potato/filesystem/json_util.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <chrono>

namespace {
    struct FrameData {
        up::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

up::Renderer::Renderer(FileSystem fileSystem, rc<gpu::GpuDevice> device) : _device(std::move(device)), _fileSystem(std::move(fileSystem)), _renderThread([this] { _renderMain(); }) {
    _commandList = _device->createCommandList();

    _debugLineMaterial = loadMaterialSync("resources/materials/debug_line.json");
    _debugLineBuffer = _device->createBuffer(gpu::BufferType::Vertex, 64 * 1024);
}

up::Renderer::~Renderer() {
    _taskQueue.close();
    _renderThread.join();
}

void up::Renderer::_renderMain() {
    RenderTask task;
    while (_taskQueue.dequeWait(task)) {
        task();
    }
}

void up::Renderer::beginFrame() {
    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(gpu::BufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) / 1000000000.0;
    FrameData frame = {
        _frameCounter++,
        static_cast<float>(now - _frameTimestamp),
        now};
    _frameTimestamp = now;

    _commandList->clear();
    _commandList->update(_frameDataBuffer.get(), view<byte>{reinterpret_cast<byte*>(&frame), sizeof(frame)});
    _commandList->bindConstantBuffer(0, _frameDataBuffer.get(), gpu::ShaderStage::All);
}

void up::Renderer::endFrame(float frameTime) {
    if (_debugLineBuffer == nullptr) {
        _debugLineBuffer = _device->createBuffer(gpu::BufferType::Vertex, 64 * 1024);
    }

    uint32 debugVertexCount = 0;
    dumpDebugDraw([this, &debugVertexCount](auto debugVertices) {
        if (debugVertices.empty()) {
            return;
        }

        _commandList->update(_debugLineBuffer.get(), debugVertices.as_bytes());
        debugVertexCount = static_cast<uint32>(debugVertices.size());
    });

    auto ctx = context();
    _debugLineMaterial->bindMaterialToRender(ctx);
    _commandList->bindVertexBuffer(0, _debugLineBuffer.get(), sizeof(DebugDrawVertex));
    _commandList->setPrimitiveTopology(gpu::PrimitiveTopology::Lines);
    _commandList->draw(debugVertexCount);

    flushDebugDraw(frameTime);

    _commandList->finish();
    _device->execute(_commandList.get());
}

auto up::Renderer::context() -> RenderContext {
    return RenderContext{
        _frameTimestamp,
        *_commandList,
        *_device};
}

auto up::Renderer::loadMeshSync(zstring_view path) -> rc<Mesh> {
    vector<byte> contents;
    auto stream = _fileSystem.openRead(path);
    if (readBinary(stream, contents) != IOResult{}) {
        return {};
    }
    stream.close();

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(contents.data(), contents.size(), 0, "assbin");
    if (scene == nullptr) {
        zstring_view error = importer.GetErrorString();
        return {};
    }
    aiMesh const* mesh = scene->mMeshes[0];

    MeshChannel channels[] = {
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Position},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Color},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::TexCoord},
    };

    uint16 stride = sizeof(float) * 8;
    uint32 size = mesh->mNumVertices * stride;

    MeshBuffer bufferDesc = {size, 0, stride};

    vector<uint16> indices;
    indices.reserve(mesh->mNumFaces * 3);

    vector<float> data;
    data.reserve(mesh->mNumVertices);

    for (uint32 i = 0; i != mesh->mNumFaces; ++i) {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    for (uint32 i = 0; i != mesh->mNumVertices; ++i) {
        data.push_back(mesh->mVertices[i].x);
        data.push_back(mesh->mVertices[i].y);
        data.push_back(mesh->mVertices[i].z);
        if (mesh->GetNumColorChannels() >= 1) {
            data.push_back(mesh->mColors[0][i].r);
            data.push_back(mesh->mColors[0][i].g);
            data.push_back(mesh->mColors[0][i].b);
        }
        else {
            data.push_back(1.f);
            data.push_back(1.f);
            data.push_back(1.f);
        }
        data.push_back(mesh->mTextureCoords[0][i].x);
        data.push_back(mesh->mTextureCoords[0][i].y);
    }

    return up::new_shared<Mesh>(std::move(indices), vector(span(data).as_bytes()), span{&bufferDesc, 1}, channels);
}

auto up::Renderer::loadMaterialSync(zstring_view path) -> rc<Material> {
    std::ifstream inFile(path.c_str());
    if (!inFile) {
        return nullptr;
    }

    auto jsonRoot = nlohmann::json::parse(inFile);
    inFile.close();

    rc<Shader> vertex;
    rc<Shader> pixel;
    vector<rc<GpuTexture>> textures;

    auto jsonShaders = jsonRoot["shaders"];
    if (jsonShaders.is_object()) {
        auto vertexPath = jsonShaders["vertex"].get<string>();
        auto pixelPath = jsonShaders["pixel"].get<string>();

        vertex = loadShaderSync(vertexPath);
        pixel = loadShaderSync(pixelPath);
    }

    auto jsonTextures = jsonRoot["textures"];
    for (auto jsonTexture : jsonTextures) {
        auto texturePath = jsonTexture.get<string>();

        auto tex = loadTextureSync(texturePath);
        if (!tex) {
            return nullptr;
        }

        textures.push_back(std::move(tex));
    }

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
    }

    return new_shared<Material>(std::move(vertex), std::move(pixel), std::move(textures));
}

auto up::Renderer::loadShaderSync(zstring_view path) -> rc<Shader> {
    vector<byte> contents;
    auto stream = _fileSystem.openRead(path);
    if (readBinary(stream, contents) != IOResult{}) {
        return {};
    }
    return up::new_shared<Shader>(std::move(contents));
}

auto up::Renderer::loadTextureSync(zstring_view path) -> rc<GpuTexture> {
    Stream stream = _fileSystem.openRead(path);
    if (!stream) {
        return nullptr;
    }

    auto img = loadImage(stream);
    if (img.data().empty()) {
        return nullptr;
    }

    gpu::TextureDesc desc = {};
    desc.type = gpu::TextureType::Texture2D;
    desc.format = gpu::Format::R8G8B8A8UnsignedNormalized;
    desc.width = img.header().width;
    desc.height = img.header().height;

    auto tex = _device->createTexture2D(desc, img.data());
    if (tex == nullptr) {
        return nullptr;
    }

    return new_shared<GpuTexture>(std::move(img), std::move(tex));
}
