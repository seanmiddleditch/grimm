// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d11_device.h"
#include "d3d11_buffer.h"
#include "d3d11_command_list.h"
#include "d3d11_debug_draw_renderer.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_platform.h"
#include "d3d11_resource_view.h"
#include "d3d11_sampler.h"
#include "d3d11_swap_chain.h"
#include "d3d11_texture.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <backends/imgui_impl_dx11.h>
#include <utility>

namespace up::d3d11 {

    DeviceD3D11::DeviceD3D11(
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

    DeviceD3D11::~DeviceD3D11() {
        if (_imguiInitialized) {
            ImGui_ImplDX11_Shutdown();
        }

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

    auto DeviceD3D11::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter) -> rc<GpuDevice> {
        UP_ASSERT(factory != nullptr);
        UP_ASSERT(adapter != nullptr);

        D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

        com_ptr<ID3D11Device> d3dDevice;
        com_ptr<ID3D11DeviceContext> context;
        auto const deviceResult = D3D11CreateDevice(
            adapter.get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            D3D11_CREATE_DEVICE_DEBUG,
            levels,
            1,
            D3D11_SDK_VERSION,
            out_ptr(d3dDevice),
            nullptr,
            out_ptr(context));
        if (FAILED(deviceResult)) {
            return nullptr;
        }
        if (d3dDevice == nullptr || context == nullptr) {
            return nullptr;
        }

        return new_shared<DeviceD3D11>(
            std::move(factory),
            std::move(adapter),
            std::move(d3dDevice),
            std::move(context));
    }

    auto DeviceD3D11::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
        UP_ASSERT(nativeWindow != nullptr);

        return SwapChainD3D11::createSwapChain(_factory.get(), _device.get(), nativeWindow);
    }

    auto DeviceD3D11::createCommandList(GpuPipelineState* pipelineState) -> rc<GpuCommandList> {
        return CommandListD3D11::createCommandList(_device.get(), pipelineState);
    }

    auto DeviceD3D11::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
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

    auto DeviceD3D11::createDepthStencilView(GpuResource* depthStencilBuffer) -> box<GpuResourceView> {
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

    auto DeviceD3D11::createShaderResourceView(GpuResource* resource) -> box<GpuResourceView> {
        UP_ASSERT(resource != nullptr);

        if (resource->resourceType() == GpuResourceType::Buffer) {
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
        else {
            auto d3dTexture = static_cast<TextureD3D11*>(resource);

            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = toNative(resource->format());
            if (d3dTexture->get()->QueryInterface(__uuidof(ID3D11Texture2D), nullptr)) {
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipLevels = 1;
                desc.Texture2D.MostDetailedMip = 0;
            }
            else {
                UP_UNREACHABLE("unsupported texture type");
            }

            com_ptr<ID3D11ShaderResourceView> view;
            HRESULT hr = _device->CreateShaderResourceView(d3dTexture->get().get(), &desc, out_ptr(view));
            if (!SUCCEEDED(hr)) {
                return nullptr;
            }

            return new_box<ResourceViewD3D11>(GpuViewType::SRV, view.as<ID3D11View>());
        }
    }

    void DeviceD3D11::initImgui(ImGuiContext& context) {
        UP_GUARD_VOID(!_imguiInitialized);
        _imguiInitialized = true;
        ImGui::SetCurrentContext(&context);
        ImGui_ImplDX11_Init(_device.get(), _context.get());
    }

    void DeviceD3D11::beginImguiFrame(ImGuiContext& context) {
        UP_GUARD_VOID(_imguiInitialized);
        ImGui::SetCurrentContext(&context);
        ImGui_ImplDX11_NewFrame();
    }

    void DeviceD3D11::renderImgui(ImGuiContext& context, GpuSwapChain& swapChain) {
        UP_GUARD_VOID(_imguiInitialized);

        auto const& d3dSwapChain = static_cast<SwapChainD3D11&>(swapChain);

        d3dSwapChain.bindToContext(_device.get(), _context.get());

        ImGui::SetCurrentContext(&context);
        ImDrawData& data = *ImGui::GetDrawData();
        ImGui_ImplDX11_RenderDrawData(&data);
    }

    void DeviceD3D11::renderDebugDraw(GpuCommandList& commandList) {
        if (_debugDrawer.empty()) {
            _debugDrawer = new_box<DebugDrawRendererD3D11>(*this);
        }
        _debugDrawer->render(commandList);
    }

    auto DeviceD3D11::createPipelineState(GpuPipelineStateDesc const& desc) -> rc<GpuPipelineState> {
        return PipelineStateD3D11::createGraphicsPipelineState(desc, _device.get());
    }

    auto DeviceD3D11::createBuffer(GpuBufferDesc const& desc, GpuDataDesc const& data) -> rc<GpuResource> {
        D3D11_BUFFER_DESC d3d11Desc = {};
        d3d11Desc.Usage = D3D11_USAGE_DYNAMIC;
        d3d11Desc.ByteWidth = static_cast<UINT>(desc.size);
        d3d11Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        switch (desc.type) {
            case GpuBufferType::Index:
                d3d11Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                break;
            case GpuBufferType::Vertex:
                d3d11Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                break;
            case GpuBufferType::Constant:
                d3d11Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                break;
            default:
                d3d11Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                break;
        }

        com_ptr<ID3D11Buffer> buffer;
        HRESULT hr = ([&] {
            if (!data.data.empty()) {
                D3D11_SUBRESOURCE_DATA init = {};
                init.pSysMem = data.data.data();
                return _device->CreateBuffer(&d3d11Desc, &init, out_ptr(buffer));
            }
            return _device->CreateBuffer(&d3d11Desc, nullptr, out_ptr(buffer));
        }());
        if (!SUCCEEDED(hr)) {
            return nullptr;
        }

        return new_shared<BufferD3D11>(desc.type, desc.size, std::move(buffer));
    }

    auto DeviceD3D11::createTexture2D(GpuTextureDesc const& desc, GpuDataDesc const& data) -> rc<GpuResource> {
        D3D11_TEXTURE2D_DESC nativeDesc = {};
        nativeDesc.Format = toNative(desc.format);
        nativeDesc.Width = desc.width;
        nativeDesc.Height = desc.height;
        nativeDesc.MipLevels = 1;
        nativeDesc.ArraySize = 1;
        nativeDesc.CPUAccessFlags = 0;
        nativeDesc.Usage = D3D11_USAGE_DEFAULT;
        nativeDesc.BindFlags = 0;
        nativeDesc.SampleDesc.Count = 1;
        nativeDesc.SampleDesc.Quality = 0;

        if ((desc.bind & GpuBindFlags::ShaderResource) != GpuBindFlags{}) {
            nativeDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if ((desc.bind & GpuBindFlags::RenderTarget) != GpuBindFlags{}) {
            nativeDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        if ((desc.bind & GpuBindFlags::DepthStencil) != GpuBindFlags{}) {
            nativeDesc.BindFlags |= D3D10_BIND_DEPTH_STENCIL;
        }

        com_ptr<ID3D11Texture2D> texture;
        HRESULT hr = ([&]() {
            if (!data.data.empty()) {
                UP_ASSERT(data.pitch * desc.height <= data.data.size());

                D3D11_SUBRESOURCE_DATA init = {};
                init.pSysMem = data.data.data();
                init.SysMemPitch = data.pitch;
                return _device->CreateTexture2D(&nativeDesc, &init, out_ptr(texture));
            }
            return _device->CreateTexture2D(&nativeDesc, nullptr, out_ptr(texture));
        }());
        if (!SUCCEEDED(hr)) {
            return nullptr;
        }

        return new_shared<TextureD3D11>(std::move(texture).as<ID3D11Resource>());
    }

    auto DeviceD3D11::createSampler(GpuSamplerDesc const& desc) -> rc<GpuSampler> {
        D3D11_SAMPLER_DESC nativeDesc = {};
        nativeDesc.AddressU = toNative(desc.address);
        nativeDesc.AddressV = nativeDesc.AddressU;
        nativeDesc.AddressW = nativeDesc.AddressU;
        nativeDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        nativeDesc.Filter = toNative(desc.filter);
        nativeDesc.MaxAnisotropy = 1;
        nativeDesc.MaxLOD = 9999;
        nativeDesc.MinLOD = 0;

        com_ptr<ID3D11SamplerState> sampler;
        HRESULT hr = _device->CreateSamplerState(&nativeDesc, out_ptr(sampler));
        if (!SUCCEEDED(hr)) {
            return nullptr;
        }

        return new_shared<SamplerD3D11>(std::move(sampler));
    }

    void DeviceD3D11::execute(GpuCommandList* commandList) {
        UP_ASSERT(commandList != nullptr);

        auto deferred = static_cast<CommandListD3D11*>(commandList);

        UP_ASSERT(deferred->commandList(), "Command list is still open");

        _context->ExecuteCommandList(deferred->commandList().get(), FALSE);
    }
} // namespace up::d3d11
