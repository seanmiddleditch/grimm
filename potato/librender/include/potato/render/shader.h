// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset.h"
#include "potato/spud/int_types.h"
#include "potato/spud/vector.h"

namespace up {
    class AssetLoader;
    class GpuDevice;
    class RenderContext;

    class Shader : public AssetBase<Shader> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.shader"_zsv;

        explicit Shader(AssetKey key, vector<byte> shader) noexcept
            : AssetBase(std::move(key))
            , _content(std::move(shader)) { }

        view<byte> content() const noexcept { return _content; }

        static UP_RENDER_API void registerLoader(AssetLoader& assetLoader, GpuDevice& device);

    private:
        vector<byte> _content;
    };
} // namespace up
