//
// Created by ivan on 7/22/26.
//

#include "../../../../Include/Render/Graph/Pass/CompositePass.h"

#include "AssetLoader/PathResolve.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RenderContext.h"

namespace RTGDEngine {
    const char *CompositePass::Name() const {
        return "CompositePass";
    }

    void CompositePass::Execute(RenderContext &context) {
        using namespace Diligent;

        const MaterialData &matData = RenderResourceManager::Instance().GetMaterial(m_material);
        if (!matData.PSO || !matData.SRB)
            return;

        auto &g = *context.Graph;
        if (auto *var = matData.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SceneColor")) {
            var->Set(g.SRV(m_sceneColor), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        }

        auto *pRTV = g.RTV(m_backBuffer);
        context.Context.SetRenderTargets(1, &pRTV, nullptr, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        DrawAttribs draw;
        draw.NumVertices = 3;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        context.Context.Draw(draw);
    }

    void CompositePass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) {
        m_material = PipelineFactory::CreateCompositePipeline(device, swapChain, GetAbsolutePath("Shaders"));
    }

    void CompositePass::Setup(RGBuilder &builder) {
        IRenderPass::Setup(builder);

        m_sceneColor = builder.Read("SceneColor");
        m_backBuffer = builder.WriteColor("Backbuffer");
    }
} // RTGDEngine
