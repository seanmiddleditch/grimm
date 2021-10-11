// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "pixel_format.h"

#include "potato/spud/int_types.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
}

namespace up {
    struct ImageHeader {
        PixelFormat pixelFormat = PixelFormat::Unknown;
        int width = 0, height = 0, stride = 0;
    };

    class Image {
    public:
        Image() = default;
        Image(ImageHeader header, vector<byte> data) noexcept : _data(std::move(data)), _header(header) {}

        ImageHeader const& header() const noexcept { return _header; }
        vector<byte> const& data() const noexcept { return _data; }

        bool isCompact() const noexcept { return _header.width == _header.stride; }
        bool isCompressed() const noexcept { return up::isCompressed(_header.pixelFormat); }

    private:
        vector<byte> _data;
        ImageHeader _header;
    };

    UP_RENDER_API Image loadImage(Stream& stream);
} // namespace up
