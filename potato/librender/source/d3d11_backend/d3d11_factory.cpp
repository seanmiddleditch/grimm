// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_factory.h"
#include "d3d11_device.h"
#include "d3d11_platform.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

up::d3d11::FactoryD3D11::FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory) : _dxgiFactory(std::move(dxgiFactory)) {
    UP_ASSERT(_dxgiFactory != nullptr);
}

up::d3d11::FactoryD3D11::~FactoryD3D11() = default;

#if defined(UP_GPU_ENABLE_D3D11)
auto up::CreateFactoryD3D11() -> box<GpuDeviceFactory> {
    com_ptr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory2), out_ptr(dxgiFactory));
    return new_box<d3d11::FactoryD3D11>(std::move(dxgiFactory));
}
#endif

bool up::d3d11::FactoryD3D11::isEnabled() const {
    return true;
}

void up::d3d11::FactoryD3D11::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    com_ptr<IDXGIAdapter1> adapter;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        GpuDeviceInfo info = {index};
        callback(info);
    }
}

auto up::d3d11::FactoryD3D11::createDevice(int index) -> rc<GpuDevice> {
    com_ptr<IDXGIAdapter1> adapter;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        if (targetIndex == index) {
            return DeviceD3D11::createDevice(_dxgiFactory, std::move(adapter));
        }
    }

    return nullptr;
}
