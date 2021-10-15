// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_pipeline_state.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/rc.h"

namespace up::d3d12 {

    class CommandListD3D12;
    class DescriptorHeapD3D12;

    enum RootSignatureType { ImGui, Model, Max };

    class RootSignatureD3D12 : public shared<RootSignatureD3D12> {
    public:

        enum RootParamIndex { ConstantBuffer, TextureSRV, TextureSampler, RootParamCount };

        explicit RootSignatureD3D12();
        virtual ~RootSignatureD3D12();

        // placeholder management of different root managers
        // @todo: come up with better way of managing root signatures during runtime
        static bool initializeSignatures(ID3D12Device* device);
        static bool destroySignatures();
        static rc<RootSignatureD3D12> getRootSignature(RootSignatureType type);

        static rc<RootSignatureD3D12> createRootSignature(ID3D12Device* device);

        ID3D12RootSignature* signature() const { return _signature.get(); }

    private:

        bool create(ID3D12Device* device);

    private:

        ID3DRootSignaturePtr _signature;
    };
} // namespace up::d3d12
