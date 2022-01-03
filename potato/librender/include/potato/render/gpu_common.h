// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/flags.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"

namespace up {
    enum class GpuFormat {
        Unknown,
        R32G32B32A32Float,
        R32G32B32Float,
        R32G32Float,
        R8G8B8A8UnsignedNormalized,
        R8G8UnsignedNormalized,
        R8UnsignedNormalized,
        D32Float
    };

    enum class GpuShaderSemantic { Position, Color, Normal, Tangent, TexCoord };

    struct GpuInputLayoutElement {
        GpuFormat format = GpuFormat::Unknown;
        GpuShaderSemantic semantic = GpuShaderSemantic::Position;
        uint32 semanticIndex = 0;
        uint32 slot = 0;
    };

    enum class GpuViewType { RTV, UAV, SRV, DSV };

    enum class GpuBufferType {
        Constant,
        Index,
        Vertex,
    };

    enum class GpuResourceType { Buffer, Texture };

    UP_DEFINE_FLAGS(GpuBindFlags, uint8_t, ShaderResource = 1 << 0, RenderTarget = 1 << 2, DepthStencil = 1 << 4);

    enum class GpuIndexFormat { Unsigned16, Unsigned32 };

    UP_DEFINE_FLAGS(GpuShaderStage, uint8_t, Vertex = 1 << 0, Pixel = 1 << 1, All = Vertex | Pixel);

    enum class GpuPrimitiveTopology {
        Triangles,
        Lines,
    };

    enum class GpuTextureAddressMode { Wrap, Clamp };

    enum class GpuFilter {
        MinMagMip_Point,
        MinMag_Point_Mip_Linear,
        MinMag_Linear_Mip_Point,
        MinMagMip_Linear,
        Anisotropic
    };

    struct GpuClipRect {
        uint32 left = 0, top = 0, right = 0, bottom = 0;
    };

    struct GpuViewportDesc {
        float leftX = 0, topY = 1;
        float width = 0, height = 0;
        float minDepth = 0, maxDepth = 1;
    };

    struct GpuDeviceInfo {
        int index;
    };

    struct GpuBufferDesc {
        GpuBufferType type = GpuBufferType::Constant;
        uint32 size = 0;
    };

    struct GpuTextureDesc {
        GpuFormat format = GpuFormat::Unknown;
        GpuBindFlags bind = GpuBindFlags::ShaderResource;
        uint32 width = 0, height = 0, depth = 0;
    };

    struct GpuSamplerDesc {
        GpuTextureAddressMode address = GpuTextureAddressMode::Clamp; // separate uvw? uses?
        GpuFilter filter = GpuFilter::MinMagMip_Linear;
    };

    struct GpuDataDesc {
        span<byte const> data;
        int pitch = 1;
    };

    struct GpuPipelineStateDesc {
        bool enableScissor = false;
        bool enableDepthWrite = false;
        bool enableDepthTest = false;
        span<byte const> vertShader;
        span<byte const> pixelShader;
        span<GpuInputLayoutElement const> inputLayout;
    };

} // namespace up
