// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/mesh.h"

#include "potato/flatbuffer/model_generated.h"
#include "potato/render/context.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/material.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/sequence.h"

#include <glm/vec3.hpp>

namespace up {
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

        class MeshLoader : public AssetLoaderBackend {
        public:
            explicit MeshLoader(GpuDevice& device) noexcept : _device(device) { }

            zstring_view typeName() const noexcept override { return Mesh::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Mesh::createFromBuffer(_device, ctx.key, contents);
            }

        private:
            GpuDevice& _device;
        };
    } // namespace

    Mesh::Mesh(
        AssetKey key,
        rc<GpuResource> ibo,
        rc<GpuResource> vbo,
        rc<GpuResource> cbo,
        view<MeshBuffer> buffers,
        view<MeshChannel> channels,
        uint32 indexCount)
        : AssetBase(std::move(key))
        , _ibo(std::move(ibo))
        , _vbo(std::move(vbo))
        , _cbo(std::move(cbo))
        , _buffers(buffers.begin(), buffers.end())
        , _channels(channels.begin(), channels.end())
        , _indexCount(indexCount) { }

    Mesh::~Mesh() = default;

    void Mesh::populateLayout(span<GpuInputLayoutElement>& inputLayout) const noexcept {
        auto size = inputLayout.size() < _channels.size() ? inputLayout.size() : _channels.size();

        for (auto index : sequence(size)) {
            inputLayout[index].format = _channels[index].format;
            inputLayout[index].semantic = _channels[index].semantic;
            inputLayout[index].semanticIndex = 0; // FIXME
            inputLayout[index].slot = _channels[index].buffer;
        }

        inputLayout = inputLayout.first(size);
    }

    auto Mesh::createFromBuffer(GpuDevice& device, AssetKey key, view<byte> buffer) -> rc<Mesh> {
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

        vector<VertData> vertices;
        vertices.reserve(numVertices);

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
            VertData& vert = vertices.emplace_back();

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

        auto indexBuffer = device.createBuffer(
            {.type = GpuBufferType::Index, .size = static_cast<uint>(indices.size() * sizeof(uint16))},
            {.data = indices.as_bytes()});
        auto vertexBuffer = device.createBuffer(
            {.type = GpuBufferType::Vertex, .size = static_cast<uint>(vertices.size() * sizeof(VertData))},
            {.data = vertices.as_bytes()});
        auto transformBuffer = device.createBuffer({.type = GpuBufferType::Constant, .size = sizeof(Trans)});

        return new_shared<Mesh>(
            std::move(key),
            std::move(indexBuffer),
            std::move(vertexBuffer),
            std::move(transformBuffer),
            span{&bufferDesc, 1},
            channels,
            static_cast<uint32>(indices.size()));
    }

    void UP_VECTORCALL Mesh::render(RenderContext& ctx, Material* material, glm::mat4x4 transform) {
        auto trans = Trans{
            .modelWorld = transpose(transform),
            .worldModel = glm::inverse(transform),
        };

        ctx.commandList().update(_cbo.get(), span{&trans, 1}.as_bytes());

        if (material != nullptr) {
            material->bindMaterialToRender(ctx);
        }

        ctx.commandList().bindIndexBuffer(_ibo.get(), GpuIndexFormat::Unsigned16, 0);
        for (auto i : sequence(_buffers.size())) {
            ctx.commandList()
                .bindVertexBuffer(static_cast<uint32>(i), _vbo.get(), _buffers[i].stride, _buffers[i].offset);
        }
        ctx.commandList().bindConstantBuffer(2, _cbo.get(), GpuShaderStage::All);
        ctx.commandList().setPrimitiveTopology(GpuPrimitiveTopology::Triangles);
        ctx.commandList().drawIndexed(static_cast<uint32>(indexCount()));
    }

    void Mesh::registerLoader(AssetLoader& assetLoader, GpuDevice& device) {
        assetLoader.registerBackend(new_box<MeshLoader>(device));
    }

} // namespace up
