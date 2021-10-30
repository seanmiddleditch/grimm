// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/platform_windows.h"
#include "potato/spud/zstring_view.h"
#include "potato/runtime/com_ptr.h"

#include <d3d12.h>
#include <dxgi1_6.h>    

#include <D3D12MemAlloc.h>

// helper global defines 
using IDXGIFactoryType = IDXGIFactory4;
using IDXGIFactoryPtr = up::com_ptr<IDXGIFactoryType>;
using IDXGIAdapterType = IDXGIAdapter1;
using IDXGIAdapterPtr = up::com_ptr<IDXGIAdapterType>;
using IDXGISwapChainType = IDXGISwapChain3;
using IDXGISwapChainPtr = up::com_ptr<IDXGISwapChainType>;

using ID3DDeviceType = ID3D12Device;
using ID3DDevicePtr = up::com_ptr<ID3D12Device>;
using ID3DResourceType = ID3D12Resource;
using ID3DResourcePtr = up::com_ptr<ID3D12Resource>;

using ID3DCommandAllocatorPtr = up::com_ptr<ID3D12CommandAllocator>;

using ID3DCommandListType = ID3D12GraphicsCommandList; 
using ID3DCommandListPtr = up::com_ptr<ID3DCommandListType>;
using ID3DCommandQueuePtr = up::com_ptr<ID3D12CommandQueue>;
using ID3DDescriptorHeapPtr = up::com_ptr<ID3D12DescriptorHeap>;
using ID3DPipelineStatePtr = up::com_ptr<ID3D12PipelineState>;
using ID3DRootSignaturePtr = up::com_ptr<ID3D12RootSignature>;
using ID3DFencePtr = up::com_ptr<ID3D12Fence>;


namespace up::d3d12 {
    extern zstring_view toNative(GpuShaderSemantic semantic) noexcept;
    extern uint32 toByteSize(GpuShaderSemantic sementic) noexcept;
    extern DXGI_FORMAT toNative(GpuFormat format) noexcept;
    extern GpuFormat fromNative(DXGI_FORMAT format) noexcept;
    extern uint32 toByteSize(GpuFormat format) noexcept;
    extern DXGI_FORMAT toNative(GpuIndexFormat type) noexcept;
} // namespace up::d3d12
