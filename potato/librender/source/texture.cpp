// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/texture.h"

#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/unique_resource.h"

#include <stb_image.h>

static int stb_read(void* user, char* data, int size) {
    auto* stream = static_cast<up::Stream*>(user);

    auto bytes = up::span<char>{data, static_cast<size_t>(size)}.as_bytes();
    if (stream->read(bytes) != up::IOResult::Success) {
        return 0;
    }

    return static_cast<int>(bytes.size());
}

static void stb_skip(void* user, int n) {
    auto* stream = static_cast<up::Stream*>(user);

    stream->seek(up::Stream::Seek::Current, n);
}

static int stb_eof(void* user) {
    auto* stream = static_cast<up::Stream*>(user);
    return stream->isEof() ? 1 : 0;
}

static constexpr stbi_io_callbacks stb_io = {&stb_read, &stb_skip, &stb_eof};

namespace up {
    namespace {
        class TextureLoader : public AssetLoaderBackend {
        public:
            explicit TextureLoader(GpuDevice& device) : _device(device) { }

            zstring_view typeName() const noexcept override { return Texture::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                int width = 0;
                int height = 0;
                int channels = 0;

                unique_resource<stbi_uc*, free> image(
                    stbi_load_from_callbacks(&stb_io, &ctx.stream, &width, &height, &channels, 4));
                ctx.stream.close();
                if (image == nullptr) {
                    return {};
                }

                GpuTextureDesc desc = {};
                desc.usage = GpuUsage::ShaderResource;
                switch (channels) {
                    case 1:
                        desc.format = GpuFormat::R8UnsignedNormalized;
                        break;
                    case 2:
                        desc.format = GpuFormat::R8G8UnsignedNormalized;
                        break;
                    case 3:
                    case 4:
                        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
                        break;
                }
                desc.width = width;
                desc.height = height;

                GpuDataDesc data;
                data.data = span{image.get(), static_cast<size_t>(width * height * channels)}.as_bytes();
                data.pitch = width * channels;

                auto tex = _device.createTexture2D(desc, data);
                if (tex == nullptr) {
                    return nullptr;
                }

                auto srv = _device.createShaderResourceView(tex.get());
                if (srv == nullptr) {
                    return nullptr;
                }

                return new_shared<Texture>(ctx.key, std::move(tex), std::move(srv));
            }

        private:
            GpuDevice& _device;
        };
    } // namespace

    Texture::Texture(AssetKey key, rc<GpuResource> texture, box<GpuResourceView> srv)
        : AssetBase(std::move(key))
        , _texture(std::move(texture))
        , _srv(std::move(srv)) { }

    Texture::~Texture() = default;

    void Texture::registerLoader(AssetLoader& assetLoader, GpuDevice& device) {
        assetLoader.registerBackend(new_box<TextureLoader>(device));
    }

} // namespace up
