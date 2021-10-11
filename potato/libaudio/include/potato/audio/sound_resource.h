// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset.h"
#include "potato/spud/rc.h"

namespace up {
    class SoundResource : public AssetBase<SoundResource> {
    public:
        using AssetBase::AssetBase;
        static constexpr zstring_view assetTypeName = "potato.asset.sound"_zsv;

    protected:
    };

    using SoundHandle = SoundResource::Handle;
} // namespace up
