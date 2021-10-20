// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_pipeline_state.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/rc.h"
#include "potato/spud/box.h"

namespace up::d3d12 {

    class CommandListD3D12;
    class DescriptorHeapD3D12;
    class RootSignatureD3D12; 

    class PipelineStateD3D12 : public GpuPipelineState {
    public:

        explicit PipelineStateD3D12();
        virtual ~PipelineStateD3D12();

        static box<PipelineStateD3D12> createGraphicsPipelineState(ID3D12Device* device, GpuPipelineStateDesc const& desc);

        bool create(ID3D12Device* device, GpuPipelineStateDesc const& desc);

        void setHeaps(ID3D12DescriptorHeap* dsvHeap, ID3D12DescriptorHeap* samplerHeap, ID3D12DescriptorHeap* rtHeap) {
            _samplerHeap = samplerHeap;
            _rtvHeap = rtHeap;
            _dsvHeap = dsvHeap;
        }

        void bindPipeline(ID3D12GraphicsCommandList* cmd);
        void bindTexture(ID3D12GraphicsCommandList* cmd, uint32 offset, D3D12_GPU_DESCRIPTOR_HANDLE srv, D3D12_GPU_DESCRIPTOR_HANDLE sampler);
        void bindConstBuffer(ID3D12GraphicsCommandList* cmd, uint32 offset, D3D12_GPU_VIRTUAL_ADDRESS cbv);
        void bindConstValues(ID3D12GraphicsCommandList* cmd, uint32 size, float* values);

        ID3D12PipelineState* state() const { return _state.get(); }

        DescriptorHeapD3D12* descHeap() const { return _srvHeap.get(); }

    private:

        ID3DPipelineStatePtr _state;
        rc<RootSignatureD3D12> _signature;

        // pipeline owned descriptor heap for shader resources
        box<DescriptorHeapD3D12> _srvHeap;

        // non-owned (external) descriptor heaps
        ID3D12DescriptorHeap* _samplerHeap;
        ID3D12DescriptorHeap* _rtvHeap;
        ID3D12DescriptorHeap* _dsvHeap;
    };
} // namespace up::d3d12
