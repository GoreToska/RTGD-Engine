//
// Created by ivan on 10/15/26.
//

#include "Render/Graph/Pass/UIPass.h"

#include "Render/Graph/RenderContext.h"
#include "UI/UISystem.h"

namespace RTGDEngine
{
    const char* UIPass::Name() const
    {
        return "UIPass";
    }

    void UIPass::Initialize(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain)
    {
    }

    void UIPass::Execute(RenderContext& context)
    {
        auto* rtv = context.Graph->RTV(m_backBuffer);
        context.Context.SetRenderTargets(1, &rtv, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        GUI().Render(context.Context);
    }

    void UIPass::Setup(RGBuilder& builder)
    {
        IRenderPass::Setup(builder);
        m_backBuffer = builder.WriteColor("Backbuffer");
    }
} // RTGDEngine
