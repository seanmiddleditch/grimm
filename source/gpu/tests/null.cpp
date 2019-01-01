#include "doctest.h"

#include "null_backend/null_objects.h"

DOCTEST_TEST_SUITE("[grimm][gpu] NullDevice") {
    DOCTEST_TEST_CASE("factory enumerates") {
        using namespace gm;

        NullFactory factory;

        DOCTEST_CHECK(factory.isEnabled());

        bool enumerated = false;
        factory.enumerateDevices([&](auto const& deviceInfo) {
            DOCTEST_CHECK_EQ(deviceInfo.index, 0);

            // ensure we only get one
            DOCTEST_CHECK_FALSE(enumerated);
            enumerated = true;
        });
    }

    DOCTEST_TEST_CASE("factory abides") {
        using namespace gm;

        NullFactory factory;

        auto device = factory.createDevice(0);
        DOCTEST_CHECK_NE(device, nullptr);
    }

    DOCTEST_TEST_CASE("device abides") {
        using namespace gm;

        NullDevice device;

        auto swapChain = device.createSwapChain(nullptr);
        DOCTEST_CHECK_NE(swapChain, nullptr);

        auto descriptorHeap = device.createDescriptorHeap();
        DOCTEST_CHECK_NE(descriptorHeap, nullptr);

        auto pipelineState = device.createPipelineState();
        DOCTEST_CHECK_NE(pipelineState, nullptr);
    }
}
