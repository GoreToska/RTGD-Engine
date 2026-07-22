//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/Pass/DebugViewPass.h"

#include "AssetLoader/PathResolve.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RenderContext.h"
#include "Render/Graph/RGResources.h"

namespace RTGDEngine {
    DebugViewPass::DebugViewPass() {
        SetEnabled(false);
    }

    void DebugViewPass::Setup(RGBuilder &builder) {
        IRenderPass::Setup(builder);

        m_backbuffer = builder.WriteColor("Backbuffer");

        switch (m_channel) {
            case EDebugChannel::Diffuse:
                m_texture = builder.Read("GBuffer.Diffuse");
                break;
            case EDebugChannel::Normal:
                m_texture = builder.Read("GBuffer.Normal");
                break;
            case EDebugChannel::Position:
                m_texture = builder.Read("GBuffer.Position");
                break;
            case EDebugChannel::PBR:
                m_texture = builder.Read("GBuffer.PBR");
                break;
            case EDebugChannel::Depth:
                m_texture = builder.Read("GBuffer.Depth");
                break;
        }
    }

    void DebugViewPass::Execute(RenderContext &context) {
        using namespace Diligent;

        auto &rm = RenderResourceManager::Instance();
        const MaterialData &mat = rm.GetMaterial(m_material);

        if (!mat.PSO || !mat.SRB) {
            return;
        }

        auto &g = *context.Graph;
        ITextureView *src = g.SRV(m_texture);

        if (!src)
            return;

        if (auto *var = mat.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Source"))
            var->Set(src, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);

        auto defTex = rm.GetDefaultTextureHandle();
        if (defTex != INVALID_TEXTURE_HANDLE) {
            const auto &tex = rm.GetTexture(defTex);
            if (auto *samVar = mat.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Sampler"))
                if (tex.Sampler)
                    samVar->Set(tex.Sampler);
        }

        auto *pRTV = g.RTV(m_backbuffer);
        context.Context.SetRenderTargets(
            1, &pRTV, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        context.Context.SetPipelineState(mat.PSO);
        context.Context.CommitShaderResources(mat.SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs draw;
        draw.NumVertices = 3;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        context.Context.Draw(draw);
    }

    void DebugViewPass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) {
        m_material = PipelineFactory::CreateDebugViewPipeline(device, swapChain, GetAbsolutePath("Shaders"));
    }

    const char *DebugViewPass::Name() const {
        return "Debug View";
    }
} // RTGDEngine
