// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_device.h"
#include "context.h"
#include "d3d12_buffer.h"
#include "d3d12_command_list.h"
#include "d3d12_context.h"
#include "d3d12_pipeline_state.h"
#include "d3d12_platform.h"
#include "d3d12_renderable.h"
#include "d3d12_resource_view.h"
#include "d3d12_sampler.h"
#include "d3d12_swap_chain.h"
#include "d3d12_texture.h"
#include "d3d12_utils.h"
#include "d3d12_desc_heap.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#define BYTE unsigned char
#include "debug_pixel_shader.h"
#include "debug_vertex_shader.h"
#undef BYTE

#include <D3D12MemAlloc.h>
#include <d3d12.h>
#include <utility>

up::d3d12::DeviceD3D12::DeviceD3D12(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter)
    : _factory(std::move(factory))
    , _adapter(std::move(adapter)) {
    UP_ASSERT(_factory != nullptr);
    UP_ASSERT(_adapter != nullptr);
}

up::d3d12::DeviceD3D12::~DeviceD3D12() {
    waitForFrame();

    _rtvHeap.reset();
//    _srvUavHeap.reset();
    _samplerHeap.reset();
    _allocator.reset();
    _device.reset();
    _adapter.reset();
    _factory.reset();
}

auto up::d3d12::DeviceD3D12::createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter) -> rc<GpuDevice> {
    UP_ASSERT(factory != nullptr);
    UP_ASSERT(adapter != nullptr);

    auto device = new_shared<DeviceD3D12>(std::move(factory), std::move(adapter));
    device->create();
    return std::move(device);
}

bool up::d3d12::DeviceD3D12::create() {
    D3D12CreateDevice(_adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), out_ptr(_device));
    if (_device == nullptr) {
        return false;
    }

    // allocate different descriptor heaps for resource views
    _rtvHeap = new_box<DescriptorHeapD3D12>(_device.get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _dsvHeap = new_box<DescriptorHeapD3D12>(_device.get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    _samplerHeap = new_box<DescriptorHeapD3D12>(_device.get(),1,D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    //#dx12 #todo: for now we will create a single Sampler for the app but in a future we should probably make it more
    // data driven or move to static sampler definitions in root signatures
    createDefaultSampler();

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    _device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), out_ptr(_commandQueue));
    SetName(_commandQueue.get(), L"commnadQueue");

    createAllocator();
    createFrameSync();

    _mainCmdList = CommandListD3D12::createCommandList(_device.get(), nullptr, D3D12_COMMAND_LIST_TYPE_DIRECT);
    _uploadCmdList = CommandListD3D12::createCommandList(_device.get(), nullptr, D3D12_COMMAND_LIST_TYPE_COPY); 

    return true;
}

auto up::d3d12::DeviceD3D12::createRenderable(IRenderable* pInterface) -> box<GpuRenderable> {
    // #dx12 #todo: use smarter allocation routine
    return new_box<RenderableD3D12>(pInterface);
}

auto up::d3d12::DeviceD3D12::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    UP_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D12::createSwapChain(_factory.get(), _device.get(), _commandQueue.get(), _rtvHeap.get(), nativeWindow);
}

auto up::d3d12::DeviceD3D12::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return CommandListD3D12::createCommandList(_device.get(), pipelineState, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

auto up::d3d12::DeviceD3D12::createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* swapChain) -> rc<GpuTexture> {
    UP_ASSERT(swapChain != nullptr);

   /* D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {_rtvHeap->get_cpu(0)};

     auto rt = static_cast<SwapChainD3D12*>(swapChain)->getBuffer(0);
     _device->CreateRenderTargetView(rt.get(), nullptr, rtvHandle);*/
    
    return new_shared<TextureD3D12>();
}
auto up::d3d12::DeviceD3D12::createPipelineState(GpuPipelineStateDesc const& desc) -> box<GpuPipelineState> {
    auto pipeline =  PipelineStateD3D12::createGraphicsPipelineState(_device.get(), desc);
    pipeline->setHeaps(nullptr, _samplerHeap->heap(), _rtvHeap->heap());
    return pipeline;
}

auto up::d3d12::DeviceD3D12::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    auto buffer = new_box<BufferD3D12>();
    ContextD3D12 ctx = {_device.get(), _mainCmdList.get(), _allocator.get()};
    buffer->create(ctx, type, size);
    return buffer;
}

auto up::d3d12::DeviceD3D12::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    auto bytesPerPixel = toByteSize(desc.format);

    UP_ASSERT(data.empty() || data.size() == desc.width * desc.height * bytesPerPixel);
    auto texture = new_shared<TextureD3D12>();

    ContextD3D12 ctx = {_device.get(), _mainCmdList.get(), _allocator.get()};
    texture->create(ctx, desc, data);

    return texture;
}

auto up::d3d12::DeviceD3D12::createSampler() -> box<GpuSampler> {
    auto sampler = new_box<SamplerD3D12>();
    sampler->create(_samplerHeap->get_gpu(0));
    return sampler;
}

auto up::d3d12::DeviceD3D12::createShaderResourceView(GpuPipelineState* pipeline, GpuTexture* resource) -> box<GpuResourceView> {
    UP_ASSERT(resource != nullptr);

    auto texture = static_cast<TextureD3D12*>(resource);
    auto ps = static_cast<PipelineStateD3D12*>(pipeline);
    auto desc_heap = ps->descHeap();

     // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = toNative(texture->format());
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    _device->CreateShaderResourceView(
        texture->get(),
        &srvDesc,
        desc_heap->heap()->GetCPUDescriptorHandleForHeapStart());

    static uint32 __srvcount = 0; 
    auto srv = new_box<ResourceViewD3D12>(GpuViewType::SRV);
    srv->create(desc_heap, __srvcount++);

    return srv;
}

auto up::d3d12::DeviceD3D12::createRenderTargetView(GpuTexture* resource) 
    -> box<GpuResourceView> {
    UP_ASSERT(resource != nullptr);

    uint32 __rtvcount = 0;
    auto rtv = new_box<ResourceViewD3D12>(GpuViewType::RTV);
    rtv->create(_rtvHeap.get(), __rtvcount++);

    return rtv;
}

auto up::d3d12::DeviceD3D12::createDepthStencilView(GpuTexture* resource)
    -> box<GpuResourceView> {
    UP_ASSERT(resource != nullptr);

    auto texture = static_cast<TextureD3D12*>(resource);
    auto desc_heap = _dsvHeap->heap();

    auto srv = new_box<ResourceViewD3D12>(GpuViewType::DSV);
    uint32 __dsvcount = 0;
    srv->create(_dsvHeap.get(), __dsvcount++);

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
    _device->CreateDepthStencilView(texture->get(), &depthStencilDesc, desc_heap->GetCPUDescriptorHandleForHeapStart());

    return srv;
}

void up::d3d12::DeviceD3D12::createDefaultSampler() {

    D3D12_SAMPLER_DESC desc = {};
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    desc.MaxAnisotropy = 1;
    desc.MaxLOD = 0;
    desc.MinLOD = 0;
    
    _device->CreateSampler(&desc, _samplerHeap->get_cpu(0));
}

void up::d3d12::DeviceD3D12::createAllocator() {
    static constexpr D3D12MA::ALLOCATOR_FLAGS allocatorFlags = D3D12MA::ALLOCATOR_FLAG_NONE;

    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = allocatorFlags;
    desc.pDevice = _device.get();
    desc.PreferredBlockSize = 0;

    // if (ENABLE_CPU_ALLOCATION_CALLBACKS) {
    //    g_AllocationCallbacks.pAllocate = &CustomAllocate;
    //    g_AllocationCallbacks.pFree = &CustomFree;
    //    g_AllocationCallbacks.pUserData = CUSTOM_ALLOCATION_USER_DATA;
    //    desc.pAllocationCallbacks = &g_AllocationCallbacks;
    //}

    D3D12MA::CreateAllocator(&desc, out_ptr(_allocator));

    switch (_allocator->GetD3D12Options().ResourceHeapTier) {
        case D3D12_RESOURCE_HEAP_TIER_1:
            wprintf(L"ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1\n");
            break;
        case D3D12_RESOURCE_HEAP_TIER_2:
            wprintf(L"ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_2\n");
            break;
        default:
            assert(0);
    }
}

void up::d3d12::DeviceD3D12::createFrameSync() {

    _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), out_ptr(_fence));
    _fenceValue = 1;

    // Create an event handle to use for frame synchronization.
    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    waitForFrame();
}

void up::d3d12::DeviceD3D12::waitForFrame() {

    const UINT64 fence = _fenceValue;
    _commandQueue->Signal(_fence.get(), fence);
    _fenceValue++;

    // Wait until the previous frame is finished.
    if (_fence->GetCompletedValue() < fence) {
        _fence->SetEventOnCompletion(fence, _fenceEvent);
        WaitForSingleObject(_fenceEvent, INFINITE);
    }
}

void up::d3d12::DeviceD3D12::beginFrame(GpuSwapChain* swapChain) {

    _mainCmdList->start(nullptr);

    swapChain->bind(_mainCmdList.get());
}

void up::d3d12::DeviceD3D12::endFrame(GpuSwapChain* swapChain) {
    swapChain->unbind(_mainCmdList.get());
}

void up::d3d12::DeviceD3D12::beginResourceCreation() {
    _mainCmdList->start(nullptr);
}

void up::d3d12::DeviceD3D12::endResourceCreation() {
    execute(false);
}

void up::d3d12::DeviceD3D12::clearCommandList() {
    execute(true);
    //_mainCmdList->start(nullptr);
}

void up::d3d12::DeviceD3D12::render(const FrameData& frameData, GpuRenderable* renderable) {
    UP_ASSERT(renderable);
    RenderContext ctx = {frameData.lastFrameTimeDelta, *_mainCmdList.get(), *this};
    static_cast<RenderableD3D12*>(renderable)->onRender(ctx);
}

void up::d3d12::DeviceD3D12::execute(bool quitting) {
    UP_ASSERT(_commandQueue != nullptr);

    if (!quitting) {

        // Close the command list and execute it to begin the initial GPU setup.
        //_uploadCmdList->finish();
        _mainCmdList->finish();

        ID3D12CommandList* ppCommandLists[] = {_mainCmdList->getResource()};
        _commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }

    // block until next frame is done on GPU before we start new frame
    waitForFrame();
}

auto up::d3d12::DeviceD3D12::getDebugShader(GpuShaderStage stage) -> view<unsigned char> {
    switch (stage) {
        case GpuShaderStage::Vertex:
            return g_vertex_main;
        case GpuShaderStage::Pixel:
            return g_pixel_main;
        default:
            return {};
    }
}

void up::d3d12::DeviceD3D12::registerAssetBackends(AssetLoader& assetLoader) {}
