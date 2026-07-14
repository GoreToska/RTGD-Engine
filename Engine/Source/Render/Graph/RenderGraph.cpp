//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/RenderGraph.h"

#include "../../../Include/Render/Graph/Pass/IRenderPass.h"

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
            if (!pass->IsEnabled()) continue;

            pass->Execute(context);
        }
    }
} // RTGDEngine
