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
#include "Tools/CameraFrustum.h"
#include "Tools/Logger.h"


namespace RTGDEngine {
    namespace {
        void CascadeGrid(const uint32_t cascadeCount, uint32_t &cols, uint32_t &rows) {
            cols = cascadeCount > 1 ? 2 : 1;
            rows = cascadeCount > 2 ? 2 : 1;
        }

        Matrix4 BuildCascadeMatrix(const CameraComponent &camera, const TransformComponent &transform,
                                   const Float3 &lightDirection, float sliceNear, float sliceFar) {
            const CameraFrustum slice = CameraFrustum::FromPerspective(
                transform.Position, transform.GetRight(), transform.GetUp(), transform.GetForward(),
                camera.FOVDegrees * Diligent::PI_F / 180.0f, camera.AspectRatio, sliceNear, sliceFar);

            const BoundingSphere bounds = slice.GetBoundingSphere();
            const float radius = std::ceil(bounds.Radius * 16.0f) / 16.0f;

            const Float3 direction = Diligent::normalize(lightDirection);
            const Float3 up = std::abs(direction.y) > 0.99f ? Float3{0, 0, 1} : Float3{0, 1, 0};

            constexpr float casterPadding = 100.0f;
            const Float3 eye = bounds.Center - direction * (radius + casterPadding);

            const Matrix4 view = LookAtLH(eye, bounds.Center, up);
            const Matrix4 projection = Matrix4::OrthoOffCenter(-radius, radius, -radius, radius, 0.0f,
                                                               2.0f * radius + casterPadding, false);

            return view * projection;
        }
    }

    void ShadowPass::Execute(RenderContext &context) {
        flecs::entity cameraEntity = CameraSystem::GetActiveCamera(context.World);
        if (!cameraEntity.is_valid()) {
            LogWarn("No camera set to render shadow pass.");
            return;
        }

        const auto *camera = cameraEntity.try_get<CameraComponent>();
        const auto *cameraTransform = cameraEntity.try_get<TransformComponent>();
        if (!camera || !cameraTransform) {
            LogWarn("No camera set to render shadow pass.");
            return;
        }

        DirectionalLightComponent light;
        context.World.each([&](const DirectionalLightComponent &l) {
            light = l;
        });

        const auto &s = RTGDRenderSystem::Instance().GetShadowSettings();
        const uint32_t cascadeCount = std::clamp(s.CascadeCount, 1u, MAX_SHADOW_CASCADES);

        uint32_t cols = 0;
        uint32_t rows = 0;
        CascadeGrid(cascadeCount, cols, rows);

        const uint32_t atlasWidth = s.Resolution * cols;
        const uint32_t atlasHeight = s.Resolution * rows;

        const float nearZ = camera->NearPlane;
        const float farZ = std::min(camera->FarPlane, s.ShadowDistance);

        ShadowConstantBuffer cb{};
        float sliceNear = nearZ;

        for (uint32_t i = 0; i < cascadeCount; ++i) {
            const float p = static_cast<float>(i + 1) / static_cast<float>(cascadeCount);
            const float logSplit = nearZ * std::pow(farZ / nearZ, p);
            const float uniformSplit = nearZ + (farZ - nearZ) * p;
            const float sliceFar = s.SplitLambda * logSplit + (1.0f - s.SplitLambda) * uniformSplit;

            cb.LightViewProjection[i] = BuildCascadeMatrix(*camera, *cameraTransform,
                                                           light.Direction, sliceNear, sliceFar);

            cb.CascadeSplits[i] = sliceFar;
            cb.AtlasRects[i] = {
                static_cast<float>(i % cols) / static_cast<float>(cols),
                static_cast<float>(i / cols) / static_cast<float>(rows),
                1.0f / static_cast<float>(cols), 1.0f / static_cast<float>(rows)
            };

            sliceNear = sliceFar;
        }

        cb.Params.x = s.DepthBias;
        cb.Params.y = s.NormalBias;
        cb.Params.z = 1.0f / static_cast<float>(s.Resolution);
        cb.Params.w = static_cast<float>(cascadeCount);

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

        for (uint32_t cascade = 0; cascade < cascadeCount; ++cascade) {
            Viewport vp = {};
            vp.TopLeftX = static_cast<float>(cascade % cols * s.Resolution);
            vp.TopLeftY = static_cast<float>(cascade / cols * s.Resolution);
            vp.Width = static_cast<float>(s.Resolution);
            vp.Height = static_cast<float>(s.Resolution);
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            context.Context.SetViewports(1, &vp, atlasWidth, atlasHeight);

            context.World.each([&](flecs::entity entity, const MeshComponent &mesh, const RenderComponent &render,
                                   TransformComponent &transform) {
                if (!render.IsVisible)
                    return;

                const MeshData &meshData = rm.GetMesh(mesh.Mesh.Handle);
                if (!meshData.VertexBuffer)
                    return;

                ObjectConstantBuffer objectCB{};
                objectCB.Model = transform.GetWorldMatrix();
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

        const auto &s = RTGDRenderSystem::Instance().GetShadowSettings();
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
