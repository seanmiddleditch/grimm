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
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class RenderContext;
    class Material;

    struct MeshBuffer {
        uint32 size = 0;
        uint32 offset = 0;
        uint16 stride = 0;
    };

    struct MeshChannel {
        uint8 buffer = 0;
        GpuFormat format = GpuFormat::R32G32B32Float;
        GpuShaderSemantic semantic = GpuShaderSemantic::Position;
    };

    class Mesh : public AssetBase<Mesh> {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.model"_zsv;

        UP_RENDER_API explicit Mesh(
            AssetKey key,
            vector<uint16> indices,
            vector<up::byte> data,
            view<MeshBuffer> buffers,
            view<MeshChannel> channels);
        UP_RENDER_API ~Mesh() override;

        UP_RENDER_API static auto createFromBuffer(AssetKey key, view<byte>) -> rc<Mesh>;

        UP_RENDER_API void populateLayout(span<GpuInputLayoutElement>& inputLayout) const noexcept;
        UP_RENDER_API void updateVertexBuffers(RenderContext& ctx);
        UP_RENDER_API void bindVertexBuffers(RenderContext& ctx);

        UP_RENDER_API void UP_VECTORCALL render(RenderContext& ctx, Material* material, glm::mat4x4 transform);

        uint32 indexCount() const noexcept { return static_cast<uint32>(_indices.size()); }

    private:
        box<GpuBuffer> _ibo;
        box<GpuBuffer> _vbo;
        box<GpuBuffer> _transformBuffer; // FIXME: this has no business being here
        vector<MeshBuffer> _buffers;
        vector<MeshChannel> _channels;
        vector<uint16> _indices;
        vector<up::byte> _data;
    };
} // namespace up
