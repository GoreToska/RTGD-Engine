//
// Created by ivan on 7/14/26.
//

#pragma once
#include <memory>
#include <vector>

#include "Pass/IRenderPass.h"
#include "Render/GBuffer.h"
#include <SwapChain.h>
#include <RenderDevice.h>

namespace RTGDEngine {
    struct RenderContext;

    class RenderGraph {
    public:
        void AddPass(std::unique_ptr<IRenderPass> pass);

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gBuffer) const;

        void Execute(RenderContext &context) const;

    private:
        std::vector<std::unique_ptr<IRenderPass> > m_passes = {};
    };
} // RTGDEngine
