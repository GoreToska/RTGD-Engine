#include "Render/RenderSystem.h"

#include <filesystem>

#include "Render/ShaderLoader.h"

#include <EngineFactoryD3D12.h>
#include <GraphicsTypes.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <PipelineState.h>
#include <Buffer.h>
#include <Shader.h>

#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Render/ConstBuffers.h"
#include "Render/RenderResourceManager.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    bool RTGDRenderSystem::Initialize(HWND hwnd, int width, int height)
    {
        using namespace Diligent;

        m_width = width;
        m_height = height;

        auto* GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
        if (!GetEngineFactoryD3D12)
        {
            LogError("Failed to load GraphicsEngineD3D12.dll");
            return false;
        }

        auto* pFactoryD3D12 = GetEngineFactoryD3D12();

        EngineD3D12CreateInfo engineCI;
        engineCI.GraphicsAPIVersion = {12, 0};

#ifdef _DEBUG
        engineCI.SetValidationLevel(VALIDATION_LEVEL_2);
#endif

        SwapChainDesc scDesc;
        scDesc.Width = width;
        scDesc.Height = height;
        scDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM_SRGB;
        scDesc.DepthBufferFormat = TEX_FORMAT_D32_FLOAT;

        Win32NativeWindow window{hwnd};

        pFactoryD3D12->CreateDeviceAndContextsD3D12(
            engineCI, &m_pDevice, &m_pImmediateContext);

        pFactoryD3D12->CreateSwapChainD3D12(
            m_pDevice, m_pImmediateContext,
            scDesc, FullScreenModeDesc{}, window, &m_pSwapChain);

        if (!m_pDevice || !m_pImmediateContext || !m_pSwapChain)
        {
            LogError("Failed to create Diligent device/swapchain");
            return false;
        }

        m_pFactory = pFactoryD3D12;

        m_initialized = true;
        InitializeConstantBuffers();

        LogInfo("Render system initialized.");
        return true;
    }

    struct TriangleVertex
    {
        float x, y;
        float r, g, b;
    };

    static const TriangleVertex kVerts[] = {
        {0.0f, 0.5f, 1.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
        {0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
    };


    void RTGDRenderSystem::InitializeConstantBuffers()
    {
        using namespace Diligent;

        BufferDesc cbDesc;
        cbDesc.Usage = USAGE_DYNAMIC;
        cbDesc.BindFlags = BIND_UNIFORM_BUFFER;
        cbDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

        cbDesc.Name = "Camera CB";
        cbDesc.Size = sizeof(CameraConstantBuffer);
        m_pDevice->CreateBuffer(cbDesc, nullptr, &m_cameraCB);

        cbDesc.Name = "Object CB";
        cbDesc.Size = sizeof(CameraConstantBuffer);
        m_pDevice->CreateBuffer(cbDesc, nullptr, &m_objectCB);

        LogInfo("Constant buffers initialized");
    }

    void RTGDRenderSystem::UpdateCameraConstantBuffer(const CameraConstantBuffer& data)
    {
        using namespace Diligent;

        void* pMapped = nullptr;
        m_pImmediateContext->MapBuffer(m_cameraCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped)
        {
            auto* dst = static_cast<CameraConstantBuffer*>(pMapped);
            dst->View = data.View.Transpose();
            dst->Projection = data.Projection.Transpose();
            dst->CameraPosition = data.CameraPosition;
            m_pImmediateContext->UnmapBuffer(m_cameraCB, MAP_WRITE);
        }
    }

    void RTGDRenderSystem::UpdateObjectConstantBuffer(const ObjectConstantBuffer& data)
    {
        using namespace Diligent;

        void* pMapped = nullptr;
        m_pImmediateContext->MapBuffer(m_objectCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped)
        {
            auto* dst = static_cast<ObjectConstantBuffer*>(pMapped);
            dst->Model = data.Model.Transpose();
            m_pImmediateContext->UnmapBuffer(m_objectCB, MAP_WRITE);
        }
    }

    void RTGDRenderSystem::Shutdown()
    {
        LogInfo("Render System Shutdown");

        m_pImmediateContext->Flush();
    }

    void RTGDRenderSystem::RenderScene(flecs::world& world)
    {
        using namespace Diligent;

        auto& rm = RenderResourceManager::Instance();

        world.each([&](const CameraComponent& cam,
                       const TransformComponent& camTransform)
        {
            CameraConstantBuffer cb;
            cb.View = cam.ViewMatrix;
            cb.Projection = cam.ProjectionMatrix;
            cb.CameraPosition = {
                camTransform.Position.x,
                camTransform.Position.y,
                camTransform.Position.z, 1.0f
            };

            UpdateCameraConstantBuffer(cb);

            world.each([&](const MeshComponent& mesh,
                           const RenderComponent& render,
                           const TransformComponent& transform)
            {
                if (!render.IsVisible)
                    return;

                const MeshData& meshData = rm.GetMesh(mesh.mesh);
                const MaterialData& matData = rm.GetMaterial(mesh.material);

                if (!meshData.VertexBuffer || !matData.PSO)
                {
                    LogWarn("Mesh or material GPU resources are null");
                    return;
                }

                ObjectConstantBuffer objCB;
                objCB.Model = transform.GetWorldMatrix();
                UpdateObjectConstantBuffer(objCB);

                m_pImmediateContext->SetPipelineState(matData.PSO);

                if (matData.SRB)
                {
                    m_pImmediateContext->CommitShaderResources(
                        matData.SRB,
                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                IBuffer* vbs[] = {meshData.VertexBuffer};
                Uint64 offsets[] = {0};
                m_pImmediateContext->SetVertexBuffers(
                    0, 1, vbs, offsets,
                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    SET_VERTEX_BUFFERS_FLAG_RESET);

                if (meshData.IndexBuffer && meshData.IndexCount > 0)
                {
                    m_pImmediateContext->SetIndexBuffer(
                        meshData.IndexBuffer, 0,
                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                    DrawIndexedAttribs draw;
                    draw.NumIndices = meshData.IndexCount;
                    draw.IndexType = VT_UINT32;
                    draw.Flags = DRAW_FLAG_NONE;
                    m_pImmediateContext->DrawIndexed(draw);
                }
                else
                {
                    DrawAttribs draw;
                    draw.NumVertices = meshData.VertexCount;
                    draw.Flags = DRAW_FLAG_NONE;
                    m_pImmediateContext->Draw(draw);
                }
            });
        });
    }

    void RTGDRenderSystem::BeginFrame()
    {
        using namespace Diligent;

        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();

        const float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};

        m_pImmediateContext->SetRenderTargets(
            1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearRenderTarget(
            pRTV, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(
            pDSV, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void RTGDRenderSystem::EndFrame()
    {
        m_pSwapChain->Present();
    }

    void RTGDRenderSystem::Resize(int width, int height)
    {
        if (!m_pSwapChain)
            return;

        m_width = width;
        m_height = height;
        m_pSwapChain->Resize(width, height);
    }
}
