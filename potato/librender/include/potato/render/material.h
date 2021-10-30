// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "shader.h"
#include "texture.h"

#include "potato/runtime/asset.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/vector.h"

namespace up {
    class AssetLoader;
    class CommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class RenderContext;

    class Material : public AssetBase<Material> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.material"_zsv;

        UP_RENDER_API explicit Material(
            AssetKey key,
            Shader::Handle vertexShader,
            Shader::Handle pixelShader,
            vector<Texture::Handle> textures);
        UP_RENDER_API ~Material() override;

        static UP_RENDER_API auto createFromBuffer(AssetKey key, view<byte> buffer, AssetLoader& assetLoader)
            -> rc<Material>;

        UP_RENDER_API void bindMaterialToRender(RenderContext& ctx);

    private:
        box<GpuPipelineState> _state;
        Shader::Handle _vertexShader;
        Shader::Handle _pixelShader;
        vector<Texture::Handle> _textures;
        vector<box<GpuResourceView>> _srvs;
        vector<box<GpuSampler>> _samplers;
    };
} // namespace up
