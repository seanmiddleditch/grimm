// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "image.h"

#include "potato/runtime/asset.h"
#include "potato/spud/rc.h"

namespace up {
    class GpuTexture;
}

namespace up {
    class Texture : public AssetBase<Texture> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.texture"_zsv;

        UP_RENDER_API explicit Texture(AssetKey key, Image image, rc<GpuTexture> texture);
        UP_RENDER_API ~Texture() override;

        GpuTexture& texture() const noexcept { return *_texture; }

    private:
        rc<GpuTexture> _texture;
        Image _image;
    };
} // namespace up
