//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/RenderGraph.h"

#include "Render/Graph/IRenderPass.h"

namespace RTGDEngine {
    void RenderGraph::AddPass(std::unique_ptr<IRenderPass> pass) {
        m_passes.push_back(std::move(pass));
    }

    void RenderGraph::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain,
                                 GBuffer &gBuffer) const {
        for (const auto &pass: m_passes) {
            pass->Initialize(device, swapChain, gBuffer);
        }
    }

    void RenderGraph::Execute(RenderContext &context) const {
        for (const auto &pass: m_passes) {
            pass->Execute(context);
        }
    }
} // RTGDEngine
