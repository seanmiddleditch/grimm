// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/render/image.h"

namespace up {
    class GpuTexture;
}

namespace up {
    class Texture : public shared<Texture> {
    public:
        UP_RENDER_API explicit Texture(Image image, rc<GpuTexture> texture);
        UP_RENDER_API ~Texture();

        GpuTexture& texture() const noexcept { return *_texture; }

    private:
        rc<GpuTexture> _texture;
        Image _image;
    };
} // namespace up
