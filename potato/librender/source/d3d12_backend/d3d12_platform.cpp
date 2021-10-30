// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_platform.h"

#include "potato/runtime/assertion.h"

auto up::d3d12::toNative(GpuShaderSemantic semantic) noexcept -> zstring_view {
    switch (semantic) {
        case GpuShaderSemantic::Position:
            return "POSITION";
        case GpuShaderSemantic::Color:
            return "COLOR";
        case GpuShaderSemantic::Normal:
            return "NORMAL";
        case GpuShaderSemantic::Tangent:
            return "TANGENT";
        case GpuShaderSemantic::TexCoord:
            return "TEXCOORD";
        default:
            UP_UNREACHABLE("Unknown Semantic");
            return "UNKNOWN";
    }
}

auto up::d3d12::toByteSize(GpuShaderSemantic sementic) noexcept -> uint32 {
    switch (sementic) {
        case GpuShaderSemantic::Position:
        case GpuShaderSemantic::Normal:
        case GpuShaderSemantic::Tangent:
            return 12;
        case GpuShaderSemantic::Color:
            return 16;
        case GpuShaderSemantic::TexCoord:
            return 8;
        default:
            UP_UNREACHABLE("Unknown Semantic");
            return 0;
    }
}
    

auto up::d3d12::toNative(GpuFormat format) noexcept -> DXGI_FORMAT {

    const DXGI_FORMAT toNativeTable[] = {
        DXGI_FORMAT_UNKNOWN,                // Unknown
        DXGI_FORMAT_R32G32B32A32_FLOAT,     // R32G32B32A32Float
        DXGI_FORMAT_R32G32B32_FLOAT,        // R32G32B32Float
        DXGI_FORMAT_R32G32_FLOAT,           // R32G32Float
        DXGI_FORMAT_R8G8B8A8_UNORM,         // R8G8B8A8UnsignedNormalized
        DXGI_FORMAT_D32_FLOAT,              // D32Float

        DXGI_FORMAT_FORCE_UINT,             // Max
    };

    return toNativeTable[format];
}

auto up::d3d12::fromNative(DXGI_FORMAT format) noexcept -> GpuFormat {
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return GpuFormat::R32G32B32A32Float;
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return GpuFormat::R32G32B32Float;
        case DXGI_FORMAT_R32G32_FLOAT:
            return GpuFormat::R32G32Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return GpuFormat::R8G8B8A8UnsignedNormalized;
        case DXGI_FORMAT_D32_FLOAT:
            return GpuFormat::D32Float;
        default:
            return GpuFormat::Unknown;
    }
}

auto up::d3d12::toByteSize(GpuFormat format) noexcept -> up::uint32 {
    switch (format) {
        case GpuFormat::R32G32B32A32Float:
            return 16;
        case GpuFormat::R32G32B32Float:
            return 12;
        case GpuFormat::R32G32Float:
            return 8;
        case GpuFormat::R8G8B8A8UnsignedNormalized:
        case GpuFormat::D32Float:
            return 4;
        default:
            UP_UNREACHABLE("Unknown Format");
            return 0;
    }
}

auto up::d3d12::toNative(GpuIndexFormat type) noexcept -> DXGI_FORMAT {
    switch (type) {
        case GpuIndexFormat::Unsigned16:
            return DXGI_FORMAT_R16_UINT;
        case GpuIndexFormat::Unsigned32:
            return DXGI_FORMAT_R32_UINT;
        default:
            UP_UNREACHABLE("Unknown GpuIndexFormat");
            return DXGI_FORMAT_UNKNOWN;
    }
}
