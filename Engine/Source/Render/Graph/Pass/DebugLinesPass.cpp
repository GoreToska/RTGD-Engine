//
// Created by ivan on 8/27/26.
//

#include "Render/Graph/Pass/DebugLinesPass.h"

#include "AssetLoader/PathResolve.h"
#include "Components/ColliderComponent.h"
#include "Components/TransformComponent.h"
#include "Render/DebugDraw.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RenderContext.h"

namespace RTGDEngine
{
    void DebugLinesPass::Execute(RenderContext& context)
    {
        context.World.each([](const ColliderComponent& collider, const TransformComponent& transform)
        {
            Float4 color = collider.IsTrigger ? Float4{1, 1, 0, 1} : Float4{0, 1, 0, 1};
            switch (collider.Shape)
            {
                case EPhysicsShape::Box:
                    GDebugDraw.DrawBox(transform.Position, collider.Extents, transform.Rotation, color);
                    break;
                case EPhysicsShape::Sphere:
                    GDebugDraw.DrawSphere(transform.Position, collider.Extents.x, color);
                    break;
                case EPhysicsShape::Capsule:
                    GDebugDraw.DrawCapsule(transform.Position, collider.Extents.y, collider.Extents.x, color);
                    break;
            }
        });

        auto lines = GDebugDraw.TakeLines();
        if (lines.empty())
            return;

        const MaterialData& mat = GRenderResources.GetMaterial(m_material);
        if (!mat.PSO || !mat.SRB)
            return;

        const auto vertexCount = static_cast<uint32_t>(lines.size());
        if (vertexCount > m_vertexCapacity)
        {
            m_vertexCapacity = vertexCount;
            m_vertexBuffer.Release();

            Diligent::BufferDesc vbDesc;
            vbDesc.Name = "DebugLines VB";
            vbDesc.Usage = Diligent::USAGE_DYNAMIC;
            vbDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
            vbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
            vbDesc.Size = m_vertexCapacity * sizeof(LineVertex);

            context.Device.CreateBuffer(vbDesc, nullptr, &m_vertexBuffer);
        }

        void* mapped = nullptr;
        context.Context.MapBuffer(m_vertexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, mapped);
        if (mapped)
        {
            memcpy(mapped, lines.data(), vertexCount * sizeof(LineVertex));
            context.Context.UnmapBuffer(m_vertexBuffer, Diligent::MAP_WRITE);
        }

        auto& g = *context.Graph;
        auto* rtv = g.RTV(m_backBuffer);
        auto* dsv = g.DSV(m_depth);
        context.Context.SetRenderTargets(1, &rtv, dsv, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        context.Context.SetPipelineState(mat.PSO);
        context.Context.CommitShaderResources(mat.SRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::IBuffer* vbs[] = {m_vertexBuffer};
        Diligent::Uint64 offsets[] = {0};
        context.Context.SetVertexBuffers(0, 1, vbs, offsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                         Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

        Diligent::DrawAttribs draw;
        draw.NumVertices = vertexCount;
        draw.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        context.Context.Draw(draw);
    }

    void DebugLinesPass::Initialize(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain)
    {
        m_material = PipelineFactory::CreateDebugLinesPipeline(device, swapChain, GetAbsolutePath("Shaders"));
    }

    const char* DebugLinesPass::Name() const
    {
        return "DebugLinesPass";
    }

    void DebugLinesPass::Setup(RGBuilder& builder)
    {
        IRenderPass::Setup(builder);

        m_backBuffer = builder.WriteColor("Backbuffer");
        m_depth = builder.WriteDepth("GBuffer.Depth");
    }
} // RTGDEngine
