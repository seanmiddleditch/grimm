// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_renderable.h"
#include "d3d12_platform.h"
#include "d3d12_utils.h"
#include "d3d12_context.h"

#include "renderer.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <d3d12.h>

up::d3d12::RenderableD3D12::RenderableD3D12(IRenderable* renderable)
: _renderable(renderable)
{}

up::d3d12::RenderableD3D12::~RenderableD3D12() = default;

bool up::d3d12::RenderableD3D12::create() {
    return true;
}

void up::d3d12::RenderableD3D12::onRender(RenderContext& ctx) {
    UP_ASSERT(_renderable != nullptr);
    _renderable->onRender(ctx);
}


