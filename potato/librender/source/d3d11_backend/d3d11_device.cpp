// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_device.h"
#include "d3d11_buffer.h"
#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_platform.h"
#include "d3d11_resource_view.h"
#include "d3d11_sampler.h"
#include "d3d11_swap_chain.h"
#include "d3d11_texture.h"

#define BYTE unsigned char
#include "debug_pixel_shader.h"
#include "debug_vertex_shader.h"
#undef BYTE

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <utility>

up::d3d11::DeviceD3D11::DeviceD3D11(
    com_ptr<IDXGIFactory2> factory,
    com_ptr<IDXGIAdapter1> adapter,
    com_ptr<ID3D11Device> device,
    com_ptr<ID3D11DeviceContext> context)
    : _factory(std::move(factory))
    , _adaptor(std::move(adapter))
    , _device(std::move(device))
    , _context(std::move(context)) {
    UP_ASSERT(_factory != nullptr);
    UP_ASSERT(_adaptor != nullptr);
    UP_ASSERT(_device != nullptr);
    UP_ASSERT(_context != nullptr);
}

up::d3d11::DeviceD3D11::~DeviceD3D11() {
    _context.reset();

    com_ptr<ID3D11Debug> debug;
    _device->QueryInterface(__uuidof(ID3D11Debug), out_ptr(debug));

    _device.reset();
    _adaptor.reset();
    _factory.reset();

    if (debug != nullptr) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
    }
}

auto up::d3d11::DeviceD3D11::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter)
    -> rc<GpuDevice> {
    UP_ASSERT(factory != nullptr);
    UP_ASSERT(adapter != nullptr);

    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

    com_ptr<ID3D11Device> device;
    com_ptr<ID3D11DeviceContext> context;
    D3D11CreateDevice(
        adapter.get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        levels,
        1,
        D3D11_SDK_VERSION,
        out_ptr(device),
        nullptr,
        out_ptr(context));
    if (device == nullptr || context == nullptr) {
        return nullptr;
    }

    return new_shared<DeviceD3D11>(std::move(factory), std::move(adapter), std::move(device), std::move(context));
}

auto up::d3d11::DeviceD3D11::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    UP_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D11::createSwapChain(_factory.get(), _device.get(), nativeWindow);
}

auto up::d3d11::DeviceD3D11::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return CommandListD3D11::createCommandList(_device.get(), pipelineState);
}

auto up::d3d11::DeviceD3D11::createRenderTargetView(GpuTexture* renderTarget) -> box<GpuResourceView> {
    UP_ASSERT(renderTarget != nullptr);

    auto d3d11Resource = static_cast<TextureD3D11*>(renderTarget);

    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    desc.Format = toNative(d3d11Resource->format());

    com_ptr<ID3D11RenderTargetView> view;
    HRESULT hr = _device->CreateRenderTargetView(d3d11Resource->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(GpuViewType::RTV, view.as<ID3D11View>());
}

auto up::d3d11::DeviceD3D11::createDepthStencilView(GpuTexture* depthStencilBuffer) -> box<GpuResourceView> {
    UP_ASSERT(depthStencilBuffer != nullptr);

    auto d3d11Resource = static_cast<TextureD3D11*>(depthStencilBuffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    desc.Format = toNative(d3d11Resource->format());

    com_ptr<ID3D11DepthStencilView> view;
    HRESULT hr = _device->CreateDepthStencilView(d3d11Resource->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(GpuViewType::DSV, view.as<ID3D11View>());
}

auto up::d3d11::DeviceD3D11::createShaderResourceView(GpuBuffer* resource) -> box<GpuResourceView> {
    UP_ASSERT(resource != nullptr);

    auto buffer = static_cast<BufferD3D11*>(resource);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.Buffer.NumElements = 0;
    desc.Buffer.ElementWidth = static_cast<UINT32>(buffer->size()) / (sizeof(float) * 4);

    com_ptr<ID3D11ShaderResourceView> view;
    HRESULT hr = _device->CreateShaderResourceView(buffer->buffer().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(GpuViewType::SRV, view.as<ID3D11View>());
}

auto up::d3d11::DeviceD3D11::createShaderResourceView(GpuTexture* texture) -> box<GpuResourceView> {
    UP_ASSERT(texture != nullptr);

    auto d3dTexture = static_cast<TextureD3D11*>(texture);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = toNative(texture->format());
    switch (texture->type()) {
        case GpuTextureType::Texture2D:
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipLevels = 1;
            desc.Texture2D.MostDetailedMip = 0;
            break;
        case GpuTextureType::Texture3D:
        case GpuTextureType::DepthStencil:
            UP_UNREACHABLE("unsupporte texture type");
        default:
            UP_UNREACHABLE("missing");
    }

    com_ptr<ID3D11ShaderResourceView> view;
    HRESULT hr = _device->CreateShaderResourceView(d3dTexture->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(GpuViewType::SRV, view.as<ID3D11View>());
}

auto up::d3d11::DeviceD3D11::createPipelineState(GpuPipelineStateDesc const& desc) -> box<GpuPipelineState> {
    return PipelineStateD3D11::createGraphicsPipelineState(desc, _device.get());
}

auto up::d3d11::DeviceD3D11::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = static_cast<UINT>(size);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    switch (type) {
        case GpuBufferType::Index:
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case GpuBufferType::Vertex:
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case GpuBufferType::Constant:
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
        default:
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
    }

    com_ptr<ID3D11Buffer> buffer;
    HRESULT hr = _device->CreateBuffer(&desc, nullptr, out_ptr(buffer));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<BufferD3D11>(type, size, std::move(buffer));
}

auto up::d3d11::DeviceD3D11::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    auto bytesPerPixel = toByteSize(desc.format);

    UP_ASSERT(data.empty() || data.size() == desc.width * desc.height * bytesPerPixel);

    D3D11_TEXTURE2D_DESC nativeDesc = {};
    nativeDesc.Format = toNative(desc.format);
    nativeDesc.Width = desc.width;
    nativeDesc.Height = desc.height;
    nativeDesc.MipLevels = 1;
    nativeDesc.ArraySize = 1;
    nativeDesc.CPUAccessFlags = 0;
    if (desc.type == GpuTextureType::DepthStencil) {
        nativeDesc.Usage = D3D11_USAGE_DEFAULT;
        nativeDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    }
    else {
        nativeDesc.Usage = D3D11_USAGE_DEFAULT;
        nativeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    }
    nativeDesc.SampleDesc.Count = 1;
    nativeDesc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = data.data();
    init.SysMemPitch = desc.width * bytesPerPixel;

    com_ptr<ID3D11Texture2D> texture;
    HRESULT hr = _device->CreateTexture2D(&nativeDesc, data.empty() ? nullptr : &init, out_ptr(texture));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_shared<TextureD3D11>(std::move(texture).as<ID3D11Resource>());
}

auto up::d3d11::DeviceD3D11::createSampler() -> box<GpuSampler> {
    D3D11_SAMPLER_DESC desc = {};
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.MaxAnisotropy = 1;
    desc.MaxLOD = 0;
    desc.MinLOD = 0;

    com_ptr<ID3D11SamplerState> sampler;
    HRESULT hr = _device->CreateSamplerState(&desc, out_ptr(sampler));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<SamplerD3D11>(std::move(sampler));
}

void up::d3d11::DeviceD3D11::execute(GpuCommandList* commandList) {
    UP_ASSERT(commandList != nullptr);

    auto deferred = static_cast<CommandListD3D11*>(commandList);

    UP_ASSERT(deferred->commandList(), "Command list is still open");

    _context->ExecuteCommandList(deferred->commandList().get(), FALSE);
}

auto up::d3d11::DeviceD3D11::getDebugShader(GpuShaderStage stage) -> view<unsigned char> {
    switch (stage) {
        case GpuShaderStage::Vertex:
            return g_vertex_main;
        case GpuShaderStage::Pixel:
            return g_pixel_main;
        default:
            return {};
    }
}

void up::d3d11::DeviceD3D11::registerAssetBackends(AssetLoader& assetLoader) {}
