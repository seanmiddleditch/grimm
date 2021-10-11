// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "image.h"

#include "potato/runtime/stream.h"

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

auto up::loadImage(Stream& stream) -> Image {
    ImageHeader header;

    int channels = 0;
    stbi_uc* image = stbi_load_from_callbacks(&stb_io, &stream, &header.width, &header.height, &channels, 4);
    if (image == nullptr) {
        return {};
    }

    vector<byte> data(header.width * header.width * 4);
    std::memcpy(data.data(), image, data.size());
    free(image); // NOLINT(cppcoreguidelines-no-malloc)

    switch (channels) {
        case 1:
            header.pixelFormat = PixelFormat::R8UnsignedNormalized;
            break;
        case 3:
            header.pixelFormat = PixelFormat::R8G8B8UnsignedNormalized;
            break;
        case 4:
            header.pixelFormat = PixelFormat::R8G8B8A8UnsignedNormalized;
            break;
    }

    return Image(header, std::move(data));
}
