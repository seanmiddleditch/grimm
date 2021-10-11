// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "mesh.h"
#include "context.h"
#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "material.h"
#include "model_generated.h"

#include "potato/spud/sequence.h"

#include <glm/vec3.hpp>

namespace {
    struct alignas(16) Vert {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 uv;
    };

    struct alignas(16) Trans {
        glm::mat4x4 modelWorld;
        glm::mat4x4 worldModel;
    };
} // namespace

up::Mesh::Mesh(
    AssetKey key,
    vector<up::uint16> indices,
    vector<up::byte> data,
    view<MeshBuffer> buffers,
    view<MeshChannel> channels)
    : AssetBase(std::move(key))
    , _buffers(buffers.begin(), buffers.end())
    , _channels(channels.begin(), channels.end())
    , _indices(std::move(indices))
    , _data(std::move(data)) {}

up::Mesh::~Mesh() = default;

void up::Mesh::populateLayout(span<GpuInputLayoutElement>& inputLayout) const noexcept {
    auto size = inputLayout.size() < _channels.size() ? inputLayout.size() : _channels.size();

    for (auto index : sequence(size)) {
        inputLayout[index].format = _channels[index].format;
        inputLayout[index].semantic = _channels[index].semantic;
        inputLayout[index].semanticIndex = 0; // FIXME
        inputLayout[index].slot = _channels[index].buffer;
    }

    inputLayout = inputLayout.first(size);
}

void up::Mesh::updateVertexBuffers(RenderContext& ctx) {
    if (_ibo == nullptr) {
        _ibo = ctx.device.createBuffer(GpuBufferType::Index, _indices.size() * sizeof(uint16));
        ctx.commandList.update(_ibo.get(), _indices.as_bytes(), 0);
    }
    if (_vbo == nullptr) {
        _vbo = ctx.device.createBuffer(GpuBufferType::Vertex, _data.size());
        ctx.commandList.update(_vbo.get(), _data, 0);
    }
}

void up::Mesh::bindVertexBuffers(RenderContext& ctx) {
    ctx.commandList.bindIndexBuffer(_ibo.get(), GpuIndexFormat::Unsigned16, 0);

    for (auto i : sequence(_buffers.size())) {
        ctx.commandList.bindVertexBuffer(static_cast<uint32>(i), _vbo.get(), _buffers[i].stride, _buffers[i].offset);
    }
}

auto up::Mesh::createFromBuffer(AssetKey key, view<byte> buffer) -> rc<Mesh> {
    flatbuffers::Verifier verifier(reinterpret_cast<uint8 const*>(buffer.data()), buffer.size());
    if (!schema::VerifyModelBuffer(verifier)) {
        return {};
    }

    auto flatModel = schema::GetModel(buffer.data());
    if (flatModel == nullptr) {
        return {};
    }

    auto flatMeshes = flatModel->meshes();
    if (flatMeshes == nullptr) {
        return {};
    }
    if (flatMeshes->size() == 0) {
        return {};
    }

    auto flatMesh = flatModel->meshes()->Get(0);
    if (flatMesh == nullptr) {
        return {};
    }

    MeshChannel channels[] = {
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Position},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Color},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent},
        {0, GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord},
    };

    struct VertData {
        glm::vec3 pos = {0, 0, 0};
        glm::vec3 color = {1, 1, 1};
        glm::vec3 normal = {0, 0, 0};
        glm::vec3 tangent = {0, 0, 0};
        glm::vec3 uv = {0, 0, 0};
    };

    uint16 stride = sizeof(VertData);
    uint32 numVertices = flatMesh->vertices()->size();
    uint32 size = numVertices * stride;

    MeshBuffer bufferDesc = {size, 0, stride};

    vector<uint16> indices;
    indices.reserve(flatMesh->indices()->size());

    vector<VertData> data;
    data.reserve(numVertices);

    auto flatIndices = flatMesh->indices();
    auto flatVerts = flatMesh->vertices();
    auto flatColors = flatMesh->colors();
    auto flatNormals = flatMesh->normals();
    auto flatTangents = flatMesh->tangents();
    auto flatUVs = flatMesh->uvs();

    for (uint32 i = 0; i != flatIndices->size(); ++i) {
        indices.push_back(flatIndices->Get(i));
    }

    for (uint32 i = 0; i != numVertices; ++i) {
        VertData& vert = data.emplace_back();

        auto pos = *flatVerts->Get(i);
        vert.pos.x = pos.x();
        vert.pos.y = pos.y();
        vert.pos.z = pos.z();

        if (flatColors != nullptr) {
            auto color = *flatColors->Get(i);
            vert.color.x = color.x();
            vert.color.y = color.y();
            vert.color.z = color.z();
        }

        if (flatNormals != nullptr) {
            auto norm = *flatNormals->Get(i);
            vert.normal.x = norm.x();
            vert.normal.y = norm.y();
            vert.normal.z = norm.z();
        }

        if (flatTangents != nullptr) {
            auto tangent = *flatTangents->Get(i);
            vert.tangent.x = tangent.x();
            vert.tangent.y = tangent.y();
            vert.tangent.z = tangent.z();
        }

        if (flatUVs != nullptr) {
            auto tex = *flatUVs->Get(i);
            vert.uv.x = tex.x();
            vert.uv.y = tex.y();
        }
    }

    return up::new_shared<Mesh>(
        std::move(key),
        std::move(indices),
        vector(data.as_bytes()),
        span{&bufferDesc, 1},
        channels);
}

void UP_VECTORCALL up::Mesh::render(RenderContext& ctx, Material* material, glm::mat4x4 transform) {
    if (_transformBuffer == nullptr) {
        _transformBuffer = ctx.device.createBuffer(GpuBufferType::Constant, sizeof(Trans));
    }

    auto trans = Trans{
        .modelWorld = transpose(transform),
        .worldModel = glm::inverse(transform),
    };

    updateVertexBuffers(ctx);
    ctx.commandList.update(_transformBuffer.get(), span{&trans, 1}.as_bytes());

    if (material != nullptr) {
        material->bindMaterialToRender(ctx);
    }

    bindVertexBuffers(ctx);
    ctx.commandList.bindConstantBuffer(2, _transformBuffer.get(), GpuShaderStage::All);
    ctx.commandList.setPrimitiveTopology(GpuPrimitiveTopology::Triangles);
    ctx.commandList.drawIndexed(static_cast<uint32>(indexCount()));
}
