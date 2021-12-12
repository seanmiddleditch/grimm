// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/context.h"

#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

namespace {
    struct alignas(16) CameraData {
        glm::mat4x4 worldViewProjection;
        glm::mat4x4 worldView;
        glm::mat4x4 viewProjection;
        glm::vec3 cameraPosition;
        glm::vec2 nearFar;
    };
} // namespace

namespace up {
    RenderContext::RenderContext(GpuDevice& device, GpuCommandList& commands, double frameTime)
        : _device(device)
        , _commandList(commands)
        , _frameTime(frameTime) { }

    RenderContext::~RenderContext() = default;

    void RenderContext::bindBackBuffer(rc<GpuTexture> target, rc<GpuTexture> depthStencil) {
        _backBuffer = std::move(target);
        _depthStencilBuffer = std::move(depthStencil);

        _rtv.reset();
        _dsv.reset();

        if (_backBuffer != nullptr) {
            _rtv = _device.createRenderTargetView(_backBuffer.get());

            auto const dimensions = _backBuffer->dimensions();

            GpuTextureDesc desc;
            desc.type = GpuTextureType::DepthStencil;
            desc.format = GpuFormat::D32Float;
            desc.width = static_cast<uint32>(dimensions.x);
            desc.height = static_cast<uint32>(dimensions.y);

            _depthStencilBuffer = _device.createTexture2D(desc, {});
            _dsv = _device.createDepthStencilView(_depthStencilBuffer.get());
        }
    }

    void UP_VECTORCALL RenderContext::applyCameraPerspective(glm::vec3 position, glm::vec3 forward, glm::vec3 up) {
        glm::vec3 const right = normalize(cross(forward, up));
        up = cross(right, forward);
        glm::mat4x4 const cameraMatrix = lookAtRH(position, position + forward, up);

        _applyCamera(position, cameraMatrix);
    }

    void RenderContext::applyCameraScreen() { _applyCamera(glm::vec3{}, glm::identity<glm::mat4x4>()); }

    void UP_VECTORCALL RenderContext::_applyCamera(glm::vec3 position, glm::mat4x4 cameraMatrix) {
        if (_cameraDataBuffer == nullptr) {
            _cameraDataBuffer = _device.createBuffer(GpuBufferType::Constant, sizeof(CameraData));
        }

        auto const dimensions = _backBuffer != nullptr ? _backBuffer->dimensions() : glm::ivec3{};

        GpuViewportDesc viewport;
        viewport.width = static_cast<float>(dimensions.x);
        viewport.height = static_cast<float>(dimensions.y);
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        constexpr float farZ = 400.f;
        constexpr float nearZ = .2f;
        constexpr float fovDeg = 75.f;

        auto projection = glm::perspectiveFovRH_ZO(glm::radians(fovDeg), viewport.width, viewport.height, nearZ, farZ);

        auto data = CameraData{
            .worldViewProjection = cameraMatrix * projection,
            .worldView = transpose(cameraMatrix),
            .viewProjection = transpose(projection),
            .cameraPosition = position,
            .nearFar = {nearZ, farZ},
        };

        _commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());

        constexpr glm::vec4 clearColor{0.f, 0.f, 0.1f, 1.f};

        _commandList.clearRenderTarget(_rtv.get(), clearColor);
        _commandList.clearDepthStencil(_dsv.get());
        _commandList.bindRenderTarget(0, _rtv.get());
        _commandList.bindDepthStencil(_dsv.get());
        _commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), GpuShaderStage::All);
        _commandList.setViewport(viewport);
    }

} // namespace up
