// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/texture.h"

#include "potato/render/context.h"
#include "potato/render/gpu_resource.h"

up::Texture::Texture(AssetKey key, Image image, rc<GpuResource> texture)
    : AssetBase(std::move(key))
    , _texture(std::move(texture))
    , _image(std::move(image)) { }

up::Texture::~Texture() = default;
