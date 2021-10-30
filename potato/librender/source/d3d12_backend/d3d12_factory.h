// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_factory.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d12 {
    class FactoryD3D12 final : public GpuDeviceFactory {
    public:
        FactoryD3D12(IDXGIFactoryPtr dxgiFactory);
        virtual ~FactoryD3D12();

        FactoryD3D12(FactoryD3D12&&) = delete;
        FactoryD3D12& operator=(FactoryD3D12&&) = delete;

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) override;
        rc<GpuDevice> createDevice(int index) override;

    private:
        IDXGIFactoryPtr _dxgiFactory;
    };
} // namespace up::d3d12
