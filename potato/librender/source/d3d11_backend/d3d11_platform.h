// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/render/gpu_common.h"
#include "potato/runtime/platform_windows.h"
#include "potato/spud/zstring_view.h"

#include <d3d11.h>
#include <dxgi1_2.h>

namespace up::d3d11 {
    extern zstring_view toNative(GpuShaderSemantic semantic) noexcept;
    extern DXGI_FORMAT toNative(GpuFormat format) noexcept;
    extern D3D11_TEXTURE_ADDRESS_MODE toNative(GpuTextureAddressMode address) noexcept;
    extern D3D11_FILTER toNative(GpuFilter filter) noexcept;
    extern GpuFormat fromNative(DXGI_FORMAT format) noexcept;
    extern uint32 toByteSize(GpuFormat format) noexcept;
    extern DXGI_FORMAT toNative(GpuIndexFormat type) noexcept;
} // namespace up::d3d11
