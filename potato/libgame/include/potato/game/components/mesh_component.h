// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/runtime/asset.h"

namespace up {
    struct MeshComponent {
        AssetHandle<up::Mesh> mesh;
        AssetHandle<Material> material;
    };
} // namespace up
