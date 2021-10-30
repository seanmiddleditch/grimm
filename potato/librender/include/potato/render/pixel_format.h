// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    enum class PixelFormat {
        Unknown,
        R8G8B8A8UnsignedNormalized,
        R8G8B8UnsignedNormalized,
        R8UnsignedNormalized,
        DXT5
    };

    constexpr bool isCompressed(PixelFormat format) noexcept {
        switch (format) {
            case PixelFormat::DXT5:
                return true;
            default:
                return false;
        }
    }

    constexpr int channelCount(PixelFormat format) noexcept {
        switch (format) {
            case PixelFormat::R8G8B8A8UnsignedNormalized:
            case PixelFormat::DXT5:
                return 4;
            case PixelFormat::R8G8B8UnsignedNormalized:
                return 3;
            case PixelFormat::R8UnsignedNormalized:
                return 1;
            default:
                return 0;
        }
    }

    constexpr int bytesPerPixel(PixelFormat format) noexcept {
        switch (format) {
            case PixelFormat::R8G8B8A8UnsignedNormalized:
                return 4;
            case PixelFormat::R8G8B8UnsignedNormalized:
                return 3;
            case PixelFormat::R8UnsignedNormalized:
            case PixelFormat::DXT5:
                return 1;
            default:
                return 0;
        }
    }
} // namespace up
