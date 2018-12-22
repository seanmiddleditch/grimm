// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "swap_chain.h"

namespace gm {
    class D3d12SwapChain : public ISwapChain {
    public:
        D3d12SwapChain(com_ptr<IDXGISwapChain1> swapChain);
        virtual ~D3d12SwapChain();

        D3d12SwapChain(D3d12SwapChain&&) = delete;
        D3d12SwapChain& operator=(D3d12SwapChain&) = delete;

        void present() override;

    private:
        com_ptr<IDXGISwapChain1> _swapChain;
    };
} // namespace gm
