// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/audio/sound_resource.h"
#include "potato/runtime/asset.h"

namespace up::component {
    struct Wave {
        float time = 0.f;
        float offset = 0.f;
    };

    struct Spin {
        float radians = 0.f;
    };

    struct Ding {
        float period = 1;
        float time = 0;
        AssetHandle<SoundResource> sound;
    };
} // namespace up::component
