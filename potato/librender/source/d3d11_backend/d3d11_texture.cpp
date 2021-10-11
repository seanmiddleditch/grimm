// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_texture.h"
#include "d3d11_platform.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

up::d3d11::TextureD3D11::TextureD3D11(com_ptr<ID3D11Resource> texture) : _texture(std::move(texture)) {}

up::d3d11::TextureD3D11::~TextureD3D11() = default;

auto up::d3d11::TextureD3D11::type() const noexcept -> GpuTextureType {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        return GpuTextureType::Texture2D;
    }

    UP_UNREACHABLE("could not detect texture type");
    return GpuTextureType::Texture2D;
}

auto up::d3d11::TextureD3D11::format() const noexcept -> GpuFormat {
    return fromNative(nativeFormat());
}

DXGI_FORMAT up::d3d11::TextureD3D11::nativeFormat() const noexcept {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        return desc.Format;
    }
    return DXGI_FORMAT_UNKNOWN;
}

auto up::d3d11::TextureD3D11::dimensions() const noexcept -> glm::ivec3 {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        return {desc.Width, desc.Height, 0};
    }

    UP_UNREACHABLE("could not detect texture type");
    return {0, 0, 0};
}
