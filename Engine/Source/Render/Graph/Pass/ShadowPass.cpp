//
// Created by ivan on 7/23/26.
//

#include "Render/Graph/Pass/ShadowPass.h"

#include <flecs.h>

#include "Components/CameraComponent.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Render/Graph/RenderContext.h"
#include "Systems/CameraSystem.h"


namespace RTGDEngine {
    void ShadowPass::Execute(RenderContext &context) {
        DirectionalLightComponent light;
        context.World.each([&](const DirectionalLightComponent &l) {
            light = l;
        });

        Float3 dir = light.Direction;
        Float3 center = {0, 0, 0};
        Float3 eye = center - dir * 100.0f;
        Float3 up = (std::abs(dir.y) > 0.99f) ? Float3{0, 0, 1} : Float3{0, 1, 0};
        Matrix4 view = LookAtLH(eye, center, up);
        Matrix4 projection = Matrix4::OrthoOffCenter(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 200.0f, false);
        Matrix4 viewProjection = view * projection;

        const auto &s = RTGDRenderSystem::Instance().GetShadowSettings();
        ShadowConstantBuffer cb{};
        cb.LightViewProjection[0] = viewProjection;
        cb.Params.x = s.DepthBias;
        cb.Params.y = s.NormalBias;
        cb.Params.z = 1.0f / static_cast<float>(s.Resolution);
        cb.Params.w = 1.0f; // only 1 cascade for now

        context.Frame.UpdateShadow(cb);

        using namespace Diligent;
        auto &rm = RenderResourceManager::Instance();
        const MaterialData &shadowMat = rm.GetMaterial(m_material);
        if (!shadowMat.PSO || !shadowMat.SRB)
            return;

        auto &g = *context.Graph;
        ITextureView *dsv = g.DSV(m_shadowMap);

        context.Context.SetRenderTargets(0, nullptr, dsv, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        context.Context.ClearDepthStencil(dsv, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        context.World.each([&](flecs::entity entity, const MeshComponent &mesh, const RenderComponent &render,
                               TransformComponent &transform) {
            if (!render.IsVisible)
                return;

            const MeshData &meshData = rm.GetMesh(mesh.Mesh.Handle);
            if (!meshData.VertexBuffer)
                return;

            ObjectConstantBuffer objectCB{};
            objectCB.Model = transform.GetWorldMatrix();
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

    void ShadowPass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) {
        m_material = PipelineFactory::CreateShadowPipeline(device, GetAbsolutePath("Shaders"));
    }

    const char *ShadowPass::Name() const {
        return "ShadowPass";
    }

    void ShadowPass::Setup(RGBuilder &builder) {
        IRenderPass::Setup(builder);

        const auto &s = RTGDRenderSystem::Instance().GetShadowSettings();
        m_shadowMap = builder.CreateDepth({
            "ShadowMap", s.Resolution, s.Resolution, Diligent::TEX_FORMAT_D32_FLOAT,
            Diligent::BIND_DEPTH_STENCIL | Diligent::BIND_SHADER_RESOURCE
        });
    }
} // RTGDEditor
