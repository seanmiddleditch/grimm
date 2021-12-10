// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/audio/sound_resource.h"
#include "potato/runtime/asset.h"

namespace up {
    struct DemoWaveComponent {
        float time = 0.f;
        float offset = 0.f;
    };

    struct DemoSpinComponent {
        float radians = 0.f;
    };

    struct DemoDingComponent {
        float period = 1;
        float time = 0;
        AssetHandle<SoundResource> sound;
    };
} // namespace up
