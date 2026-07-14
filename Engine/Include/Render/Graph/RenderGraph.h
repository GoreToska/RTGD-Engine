//
// Created by ivan on 7/14/26.
//

#pragma once
#include <memory>
#include <vector>

#include "IRenderPass.h"

namespace Diligent {
    struct ISwapChain;
    struct IRenderDevice;
}

namespace RTGDEngine {
    struct RenderContext;
    struct GBuffer;

    class RenderGraph {
    public:
        void AddPass(std::unique_ptr<IRenderPass> pass);

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gBuffer) const;

        void Execute(RenderContext &context) const;

    private:
        std::vector<std::unique_ptr<IRenderPass> > m_passes = {};
    };
} // RTGDEngine
