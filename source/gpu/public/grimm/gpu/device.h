// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"

namespace gm {
    class ISwapChain;
    class IDescriptorHeap;
    class IGpuResource;
    class IPipelineState;
    class ICommandList;

    class IGPUDevice {
    public:
        IGPUDevice() = default;
        virtual ~IGPUDevice() = default;

        IGPUDevice(IGPUDevice&&) = delete;
        IGPUDevice& operator=(IGPUDevice&&) = delete;

        virtual box<ISwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<IDescriptorHeap> createDescriptorHeap() = 0;
        virtual box<ICommandList> createCommandList(IPipelineState* pipelineState = nullptr) = 0;
        virtual box<IPipelineState> createPipelineState() = 0;

        virtual void execute(ICommandList* commandList) = 0;

        virtual void createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) = 0;
    };
} // namespace gm
