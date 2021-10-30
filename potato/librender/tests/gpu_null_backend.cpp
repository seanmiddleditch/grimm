// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_swap_chain.h"

#include <catch2/catch.hpp>

TEST_CASE("potato.render.DeviceNull", "[potato][gpu]") {
    using namespace up;

    SECTION("factory enumerates") {
        auto factory = CreateFactoryNull();

        CHECK(factory->isEnabled());

        bool enumerated = false;
        factory->enumerateDevices([&](auto const& deviceInfo) {
            CHECK(deviceInfo.index == 0);

            // ensure we only get one
            CHECK_FALSE(enumerated);
            enumerated = true;
        });
    }

    SECTION("factory abides") {
        auto factory = CreateFactoryNull();

        auto device = factory->createDevice(0);
        CHECK(device.get() != nullptr);
    }

    SECTION("device abides") {
        auto factory = CreateFactoryNull();
        auto device = factory->createDevice(0);

        auto swapChain = device->createSwapChain(nullptr);
        CHECK(swapChain.get() != nullptr);

        GpuPipelineStateDesc desc;
        auto pipelineState = device->createPipelineState(desc);
        CHECK(pipelineState.get() != nullptr);
    }
}
