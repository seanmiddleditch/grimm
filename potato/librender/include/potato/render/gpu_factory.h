// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "gpu_common.h"

#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/rc.h"

namespace up {
    class GpuDevice;

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) = 0;
        virtual rc<GpuDevice> createDevice(int index) = 0;
    };

    UP_RENDER_API box<GpuDeviceFactory> CreateFactoryNull();
#if defined(UP_GPU_ENABLE_D3D12)
    UP_RENDER_API box<GpuDeviceFactory> CreateFactoryD3D12();
#endif
} // namespace up
