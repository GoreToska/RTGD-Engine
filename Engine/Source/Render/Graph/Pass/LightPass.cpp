//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/Pass/LightPass.h"

#include "AssetLoader/PathResolve.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RenderContext.h"
#include "Render/Graph/RGResources.h"

namespace RTGDEngine {
    const char *LightPass::Name() const {
        return "Light";
    }

    void LightPass::Setup(RGBuilder &builder) {
        IRenderPass::Setup(builder);


    }

    void LightPass::Execute(RenderContext &context) {
        using namespace Diligent;

        auto &rm = RenderResourceManager::Instance();
        const MaterialData &matData = rm.GetMaterial(m_material);

        if (!matData.PSO || !matData.SRB)
            return;

        auto bindSRV = [&](const char *name, ITextureView *srv) {
            auto *var = matData.SRB->GetVariableByName(SHADER_TYPE_PIXEL, name);
            if (var && srv)
                var->Set(srv, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        };

        auto &g = *context.Graph;
        bindSRV("g_Diffuse", g.SRV(g.Find("GBuffer.Diffuse")));
        bindSRV("g_Normal", g.SRV(g.Find("GBuffer.Normal")));
        bindSRV("g_Position", g.SRV(g.Find("GBuffer.Position")));
        bindSRV("g_PBR", g.SRV(g.Find("GBuffer.PBR")));

        auto defTex = RenderResourceManager::Instance().GetDefaultTextureHandle();
        if (defTex != INVALID_TEXTURE_HANDLE) {
            const auto &tex = RenderResourceManager::Instance().GetTexture(defTex);
            auto *samVar = matData.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Sampler");
            if (samVar && tex.Sampler)
                samVar->Set(tex.Sampler);
        }

        auto *pRTV = g.RTV(g.Find("Backbuffer"));

        context.Context.SetRenderTargets(
            1, &pRTV, nullptr,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
        context.Context.ClearRenderTarget(
            pRTV, clearColor,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        context.Context.SetPipelineState(matData.PSO);
        context.Context.CommitShaderResources(
            matData.SRB,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs draw;
        draw.NumVertices = 3;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        context.Context.Draw(draw);
    }

    void LightPass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) {
        m_material = PipelineFactory::CreateLightingPipeline(device, swapChain, GetAbsolutePath("Shaders"));
    }
} // RTGDEngine
