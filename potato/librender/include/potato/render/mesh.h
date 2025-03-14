// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "gpu_common.h"

#include "potato/runtime/asset.h"
#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <glm/mat4x4.hpp>

namespace up {
    class CommandList;
    class GpuDevice;
    class GpuResource;
} // namespace up

namespace up {
    class AssetLoader;
    class GpuDevice;
    class RenderContext;
    class Material;

    class Mesh : public AssetBase<Mesh> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.model"_zsv;

        UP_RENDER_API explicit Mesh(
            AssetKey key,
            rc<GpuResource> ibo,
            rc<GpuResource> vbo,
            rc<GpuResource> cbo,
            uint32 indexCount);
        UP_RENDER_API ~Mesh();

        UP_RENDER_API static auto createFromBuffer(GpuDevice& device, AssetKey key, view<byte>) -> rc<Mesh>;

        UP_RENDER_API void UP_VECTORCALL render(RenderContext& ctx, Material* material, glm::mat4x4 transform);

        uint32 indexCount() const noexcept { return _indexCount; }

        static UP_RENDER_API void registerLoader(AssetLoader& assetLoader, GpuDevice& device);

    private:
        rc<GpuResource> _ibo;
        rc<GpuResource> _vbo;
        rc<GpuResource> _cbo; // FIXME: Transform buffer; this has no business being here
        uint32 _indexCount = 0;
    };
} // namespace up
