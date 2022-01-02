// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/material.h"

#include "potato/flatbuffer/material_generated.h"
#include "potato/render/context.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_sampler.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"

namespace up {
    namespace {
        class MaterialLoader : public AssetLoaderBackend {
        public:
            explicit MaterialLoader(GpuDevice& device) : _device(device) { }

            zstring_view typeName() const noexcept override { return Material::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Material::createFromBuffer(_device, ctx.key, contents, ctx.loader);
            }

        private:
            GpuDevice& _device;
        };
    } // namespace

    Material::Material(AssetKey key, rc<GpuPipelineState> pipelineState, vector<Texture::Handle> textures)
        : AssetBase(std::move(key))
        , _pipelineState(std::move(pipelineState))
        , _textures(std::move(textures)) { }

    Material::~Material() = default;

    void Material::bindMaterialToRender(RenderContext& ctx) {
        ctx.commandList().setPipelineState(_pipelineState.get());

        for (auto index : sequence(static_cast<uint32>(_textures.size()))) {
            ctx.commandList().bindShaderResource(index, &_textures[index].asset()->srv(), GpuShaderStage::Pixel);
        }
    }

    auto Material::createFromBuffer(GpuDevice& device, AssetKey key, view<byte> buffer, AssetLoader& assetLoader)
        -> rc<Material> {
        flatbuffers::Verifier verifier(reinterpret_cast<uint8 const*>(buffer.data()), buffer.size());
        if (!flat::VerifyMaterialBuffer(verifier)) {
            return {};
        }

        auto material = flat::GetMaterial(buffer.data());

        Shader::Handle vertex;
        Shader::Handle pixel;
        vector<Texture::Handle> textures;

        auto shader = material->shader();
        if (shader == nullptr) {
            return nullptr;
        }

        vertex = assetLoader.loadAssetSync<Shader>(static_cast<AssetId>(shader->vertex()->id()));
        pixel = assetLoader.loadAssetSync<Shader>(static_cast<AssetId>(shader->pixel()->id()));

        if (!vertex.ready()) {
            return nullptr;
        }

        if (!vertex.ready()) {
            return nullptr;
        }

        for (auto textureData : *material->textures()) {
            auto tex = assetLoader.loadAssetSync<Texture>(static_cast<AssetId>(textureData->id()));
            if (!tex.ready()) {
                return nullptr;
            }

            textures.push_back(std::move(tex));
        }

        GpuPipelineStateDesc pipelineDesc;

        GpuInputLayoutElement layout[] = {
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
            {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
        };

        pipelineDesc.enableDepthTest = true;
        pipelineDesc.enableDepthWrite = true;
        pipelineDesc.vertShader = vertex.asset()->content();
        pipelineDesc.pixelShader = pixel.asset()->content();
        pipelineDesc.inputLayout = layout;
        auto pipelineState = device.createPipelineState(pipelineDesc);

        return new_shared<Material>(std::move(key), std::move(pipelineState), std::move(textures));
    }

    void Material::registerLoader(AssetLoader& assetLoader, GpuDevice& device) {
        assetLoader.registerBackend(new_box<MaterialLoader>(device));
    }
} // namespace up
