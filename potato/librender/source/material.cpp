// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "material.h"
#include "context.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_pipeline_state.h"
#include "gpu_resource_view.h"
#include "gpu_sampler.h"
#include "material_generated.h"
#include "shader.h"
#include "texture.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/string.h"

up::Material::Material(
    AssetKey key,
    Shader::Handle vertexShader,
    Shader::Handle pixelShader,
    vector<Texture::Handle> textures)
    : AssetBase(std::move(key))
    , _vertexShader(std::move(vertexShader))
    , _pixelShader(std::move(pixelShader))
    , _textures(std::move(textures))
    , _srvs(_textures.size())
    , _samplers(_textures.size()) {}

up::Material::~Material() = default;

void up::Material::bindMaterialToRender(RenderContext& ctx) {
    if (_state == nullptr) {
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
        pipelineDesc.vertShader = _vertexShader.asset()->content();
        pipelineDesc.pixelShader = _pixelShader.asset()->content();
        pipelineDesc.inputLayout = layout;
        _state = ctx.device.createPipelineState(pipelineDesc);

        int texIndex = 0;
        for (auto const& tex : _textures) {
            _srvs[texIndex] = ctx.device.createShaderResourceView(&tex.asset()->texture());
            _samplers[texIndex++] = ctx.device.createSampler();
        }
    }

    ctx.commandList.setPipelineState(_state.get());

    int texIndex = 0;
    for (auto const& srv : _srvs) {
        ctx.commandList.bindSampler(texIndex, _samplers[texIndex].get(), GpuShaderStage::Pixel);
        ctx.commandList.bindShaderResource(texIndex++, srv.get(), GpuShaderStage::Pixel);
    }
}

auto up::Material::createFromBuffer(AssetKey key, view<byte> buffer, AssetLoader& assetLoader) -> rc<Material> {
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

    return new_shared<Material>(std::move(key), std::move(vertex), std::move(pixel), std::move(textures));
}
