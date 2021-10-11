// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d11_platform.h"
#include "gpu_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d11 {
    class TextureD3D11 final : public GpuTexture {
    public:
        explicit TextureD3D11(com_ptr<ID3D11Resource> texture);
        virtual ~TextureD3D11();

        TextureD3D11(TextureD3D11&&) = delete;
        TextureD3D11& operator=(TextureD3D11&&) = delete;

        GpuTextureType type() const noexcept override;
        GpuFormat format() const noexcept override;
        glm::ivec3 dimensions() const noexcept override;

        DXGI_FORMAT nativeFormat() const noexcept;
        com_ptr<ID3D11Resource> const& get() const { return _texture; }

    private:
        com_ptr<ID3D11Resource> _texture;
    };
} // namespace up::d3d11
