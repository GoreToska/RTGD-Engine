//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/Pass/GBufferPass.h"

#include <flecs.h>
#include "Render/Graph/RenderContext.h"
#include "AssetLoader/PathResolve.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RGResources.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    const char* GBufferPass::Name() const
    {
        return "GBuffer";
    }

    void GBufferPass::Setup(RGBuilder& builder)
    {
        IRenderPass::Setup(builder);

        using namespace Diligent;

        m_diffuse = builder.CreateColor({"GBuffer.Diffuse", 0, 0, TEX_FORMAT_RGBA8_UNORM_SRGB});
        m_normal = builder.CreateColor({"GBuffer.Normal", 0, 0, TEX_FORMAT_RGBA16_FLOAT});
        m_position = builder.CreateColor({"GBuffer.Position", 0, 0, TEX_FORMAT_RGBA32_FLOAT});
        m_pbr = builder.CreateColor({"GBuffer.PBR", 0, 0, TEX_FORMAT_RGBA8_UNORM});
        m_depth = builder.CreateDepth({
            "GBuffer.Depth", 0, 0, TEX_FORMAT_D32_FLOAT, BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE
        });

#ifdef RTGD_EDITOR
        m_id = builder.CreateColor({"GBuffer.ID", 0, 0, TEX_FORMAT_R32_UINT});
#endif
    }

    void GBufferPass::Initialize(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain)
    {
        m_material = PipelineFactory::CreateGBufferPipeline(device, GetAbsolutePath("Shaders"));
    }

    void GBufferPass::Execute(RenderContext& context)
    {
        using namespace Diligent;

        auto& g = *context.Graph;
        ITextureView* diffuseRTV = g.RTV(m_diffuse);
        ITextureView* normalRTV = g.RTV(m_normal);
        ITextureView* positionRTV = g.RTV(m_position);
        ITextureView* pbrRTV = g.RTV(m_pbr);
        ITextureView* depthDSV = g.DSV(m_depth);
#ifdef RTGD_EDITOR
        ITextureView* idRTV = g.RTV(m_id);
#endif

        auto& rm = GRenderResources;
        TextureHandle def = rm.GetDefaultTextureHandle();
        TextureHandle defNormal = rm.GetDefaultNormalTextureHandle();

        ITextureView* rtvs[] = {
            diffuseRTV,
            normalRTV,
            positionRTV,
            pbrRTV,
#ifdef RTGD_EDITOR
            idRTV,
#endif
        };
        context.Context.SetRenderTargets(
            std::size(rtvs), rtvs, depthDSV,
            RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        context.Context.ClearRenderTarget(diffuseRTV, clearColor,
                                          RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        context.Context.ClearRenderTarget(normalRTV, clearColor,
                                          RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        context.Context.ClearRenderTarget(positionRTV, clearColor,
                                          RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        context.Context.ClearRenderTarget(pbrRTV, clearColor,
                                          RESOURCE_STATE_TRANSITION_MODE_VERIFY);

#ifdef RTGD_EDITOR
        context.Context.ClearRenderTarget(idRTV, clearColor,
                                          RESOURCE_STATE_TRANSITION_MODE_VERIFY);
#endif

        context.Context.ClearDepthStencil(
            depthDSV, CLEAR_DEPTH_FLAG, 1.0f, 0,
            RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const MaterialData& gbufMat = rm.GetMaterial(m_material);
        if (!gbufMat.PSO || !gbufMat.SRB)
            return;

        const RenderScene& scene = *context.Scene;
        context.MainView->Mask.ForEach([&](uint32_t i)
        {
            const MeshData& meshData = rm.GetMesh(scene.Mesh()[i]);

            MaterialData& objMat = rm.GetMaterial(scene.Material()[i]);

            if (!objMat.SRB || objMat.IsDirty)
            {
                RenderResourceManager::BuildGBufferSRB(objMat, gbufMat.PSO);
            }

            ObjectConstantBuffer objectCB{};
            objectCB.Model = scene.World()[i];
#ifdef RTGD_EDITOR
            objectCB.EntityID = i + 1;
#endif
            context.Frame.UpdateObject(objectCB);

            context.Context.SetPipelineState(gbufMat.PSO);
            context.Context.CommitShaderResources(
                objMat.SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            IBuffer* vbs[] = {meshData.VertexBuffer};
            Uint64 offsets[] = {0};
            context.Context.SetVertexBuffers(
                0, 1, vbs, offsets,
                RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                SET_VERTEX_BUFFERS_FLAG_RESET);

            if (meshData.IndexBuffer && meshData.IndexCount > 0)
            {
                context.Context.SetIndexBuffer(
                    meshData.IndexBuffer, 0,
                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                DrawIndexedAttribs draw;
                draw.NumIndices = meshData.IndexCount;
                draw.IndexType = VT_UINT32;
                draw.Flags = DRAW_FLAG_VERIFY_ALL;
                context.Context.DrawIndexed(draw);
            }
            else
            {
                DrawAttribs draw;
                draw.NumVertices = meshData.VertexCount;
                draw.Flags = DRAW_FLAG_VERIFY_ALL;
                context.Context.Draw(draw);
            }
        });
    }
} // RTGDEngine
