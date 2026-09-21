//
// Created by ivan on 7/23/26.
//

#include "Render/Graph/Pass/ShadowPass.h"

#include <flecs.h>

#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Render/Graph/RenderContext.h"
#include "Render/ShadowMap/ShadowCascades.h"


namespace RTGDEngine {
    void ShadowPass::Execute(RenderContext &context) {
        if (context.ShadowViews.empty() || !context.Scene)
            return;

        const auto &s = GRenderSystem().GetShadowSettings();
        const uint32_t cascadeCount = static_cast<uint32_t>(context.ShadowViews.size());
        const RenderScene &scene = *context.Scene;

        uint32_t cols = 0;
        uint32_t rows = 0;
        CascadeGrid(cascadeCount, cols, rows);

        const uint32_t atlasWidth = s.Resolution * cols;
        const uint32_t atlasHeight = s.Resolution * rows;

        ShadowConstantBuffer cb{};

        for (uint32_t i = 0; i < cascadeCount; ++i) {
            const RenderView &view = context.ShadowViews[i];

            cb.LightViewProjection[i] = view.ViewProjection;
            cb.CascadeParams[i] = {1 / view.DepthRange, view.TexelWorldSize, 0.0f, 0.0f};

            cb.CascadeSplits[i] = view.SplitFar;
            cb.AtlasRects[i] = {
                static_cast<float>(i % cols) / static_cast<float>(cols),
                static_cast<float>(i / cols) / static_cast<float>(rows),
                1.0f / static_cast<float>(cols), 1.0f / static_cast<float>(rows)
            };
        }

        cb.Params.x = s.DepthBias;
        cb.Params.y = s.NormalBias;
        cb.Params.z = 1.0f / static_cast<float>(s.Resolution);
        cb.Params.w = static_cast<float>(cascadeCount);
        cb.Params2.x = s.CascadeBlend;
        cb.Params2.y = s.DebugCascades ? 1.0f : 0.0f;

        context.Frame.UpdateShadow(cb);

        using namespace Diligent;
        auto &rm = GRenderResources();
        const MaterialData &shadowMat = rm.GetMaterial(m_material);
        if (!shadowMat.PSO || !shadowMat.SRB)
            return;

        auto &g = *context.Graph;
        ITextureView *dsv = g.DSV(m_shadowMap);

        context.Context.SetRenderTargets(0, nullptr, dsv, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        context.Context.ClearDepthStencil(dsv, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        for (uint32_t cascade = 0; cascade < cascadeCount; ++cascade) {
            Viewport vp = {};
            vp.TopLeftX = static_cast<float>(cascade % cols * s.Resolution);
            vp.TopLeftY = static_cast<float>(cascade / cols * s.Resolution);
            vp.Width = static_cast<float>(s.Resolution);
            vp.Height = static_cast<float>(s.Resolution);
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            context.Context.SetViewports(1, &vp, atlasWidth, atlasHeight);

            context.ShadowViews[cascade].Mask.ForEach([&](uint32_t i) {
                const MeshData &meshData = rm.GetMesh(scene.Mesh()[i]);

                ObjectConstantBuffer objectCB{};
                objectCB.Model = scene.World()[i];
                objectCB.CascadeIndex = cascade;
                context.Frame.UpdateObject(objectCB);

                context.Context.SetPipelineState(shadowMat.PSO);
                context.Context.CommitShaderResources(shadowMat.SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                IBuffer *vbs[] = {meshData.VertexBuffer};
                Uint64 offsets[] = {0};
                context.Context.SetVertexBuffers(0, 1, vbs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                 SET_VERTEX_BUFFERS_FLAG_RESET);

                if (meshData.IndexBuffer && meshData.IndexCount > 0) {
                    context.Context.SetIndexBuffer(meshData.IndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                    DrawIndexedAttribs draw;
                    draw.NumIndices = meshData.IndexCount;
                    draw.IndexType = VT_UINT32;
                    draw.Flags = DRAW_FLAG_VERIFY_ALL;
                    context.Context.DrawIndexed(draw);
                } else {
                    DrawAttribs draw;
                    draw.NumVertices = meshData.VertexCount;
                    draw.Flags = DRAW_FLAG_VERIFY_ALL;
                    context.Context.Draw(draw);
                }
            });
        }
    }

    void ShadowPass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) {
        m_material = PipelineFactory::CreateShadowPipeline(device, GetAbsolutePath("Shaders"));
    }

    const char *ShadowPass::Name() const {
        return "ShadowPass";
    }

    void ShadowPass::Setup(RGBuilder &builder) {
        IRenderPass::Setup(builder);

        const auto &s = GRenderSystem().GetShadowSettings();
        const uint32_t cascadeCount = std::clamp(s.CascadeCount, 1u, MAX_SHADOW_CASCADES);

        uint32_t cols = 0;
        uint32_t rows = 0;
        CascadeGrid(cascadeCount, cols, rows);

        m_shadowMap = builder.CreateDepth({
            "ShadowAtlas", s.Resolution * cols, s.Resolution * rows, Diligent::TEX_FORMAT_D32_FLOAT,
            Diligent::BIND_DEPTH_STENCIL | Diligent::BIND_SHADER_RESOURCE
        });
    }
} // RTGDEditor
