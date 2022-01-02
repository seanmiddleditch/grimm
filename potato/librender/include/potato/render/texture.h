// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"

namespace up {
    class AssetLoader;
    class GpuDevice;
    class GpuResource;
    class GpuResourceView;

    class Texture : public AssetBase<Texture> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.texture"_zsv;

        UP_RENDER_API explicit Texture(AssetKey key, rc<GpuResource> texture, box<GpuResourceView> srv);
        UP_RENDER_API ~Texture();

        GpuResource& texture() const noexcept { return *_texture; }
        GpuResourceView& srv() const noexcept { return *_srv; }

        static UP_RENDER_API void registerLoader(AssetLoader& assetLoader, GpuDevice& device);

    private:
        rc<GpuResource> _texture;
        box<GpuResourceView> _srv;
    };
} // namespace up
