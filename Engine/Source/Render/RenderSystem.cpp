#include "Render/RenderSystem.h"

#include <filesystem>

#include "Render/ShaderLoader.h"


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
#include "Render/GBufferFactory.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Systems/CameraSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    bool RTGDRenderSystem::Initialize(const NativeWindowHandle &handle, int width, int height) {
        using namespace Diligent;

        m_width = width;
        m_height = height;

        SwapChainDesc scDesc;
        scDesc.Width = width;
        scDesc.Height = height;
        scDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM_SRGB;
        scDesc.DepthBufferFormat = TEX_FORMAT_D32_FLOAT;

#ifdef _WIN32
        //-------- D3D12 --------
        auto *GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
        if (!GetEngineFactoryD3D12) {
            LogError("Failed to load GraphicsEngineD3D12.dll");
            return false;
        }
        auto *pFactory = GetEngineFactoryD3D12();

        EngineD3D12CreateInfo engineCI;
        engineCI.GraphicsAPIVersion = {12, 0};
#ifdef _DEBUG
        engineCI.SetValidationLevel(VALIDATION_LEVEL_2);
#endif

        pFactory->CreateDeviceAndContextsD3D12(engineCI, &m_device, &m_pImmediateContext);

        Win32NativeWindow nativeWindow{static_cast<HWND>(handle.hwnd)};
        pFactory->CreateSwapChainD3D12(
            m_device, m_pImmediateContext,
            scDesc, FullScreenModeDesc{}, nativeWindow, &m_swapChain);

        m_pFactory = pFactory;

#elif defined(__linux__)
        auto *pFactory = GetEngineFactoryVk();
        if (!pFactory) {
            LogError("Failed to load GraphicsEngineVk");
            return false;
        }

        EngineVkCreateInfo engineCI;
#ifdef _DEBUG
        engineCI.SetValidationLevel(VALIDATION_LEVEL_2);
#endif

        pFactory->CreateDeviceAndContextsVk(engineCI, &m_device, &m_pImmediateContext);

        LinuxNativeWindow nativeWindow;
        nativeWindow.WindowId = handle.window;
        nativeWindow.pDisplay = handle.display;
        pFactory->CreateSwapChainVk(
            m_device, m_pImmediateContext,
            scDesc, nativeWindow, &m_swapChain);

        m_pFactory = pFactory;
#endif

        if (!m_device || !m_pImmediateContext || !m_swapChain) {
            LogError("Failed to create Diligent device/swapchain");
            return false;
        }

        m_initialized = true;
        InitializeConstantBuffers();

        m_gbuffer = GBufferFactory::Create(*m_device, width, height);
        m_gbufferMaterial = PipelineFactory::CreateGBufferPipeline(
            *m_device, m_gbuffer, "Shaders");
        m_lightingMaterial = PipelineFactory::CreateLightingPipeline(
            *m_device, *m_swapChain, "Shaders");

        LogInfo("Render system initialized.");
        return true;
    }

    struct TriangleVertex {
        float x, y;
        float r, g, b;
    };

    static const TriangleVertex kVerts[] = {
        {0.0f, 0.5f, 1.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
        {0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
    };


    void RTGDRenderSystem::InitializeConstantBuffers() {
        using namespace Diligent;

        BufferDesc cbDesc;
        cbDesc.Usage = USAGE_DYNAMIC;
        cbDesc.BindFlags = BIND_UNIFORM_BUFFER;
        cbDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

        cbDesc.Name = "Camera CB";
        cbDesc.Size = sizeof(CameraConstantBuffer);
        m_device->CreateBuffer(cbDesc, nullptr, &m_cameraCB);

        cbDesc.Name = "Object CB";
        cbDesc.Size = sizeof(CameraConstantBuffer);
        m_device->CreateBuffer(cbDesc, nullptr, &m_objectCB);

        cbDesc.Name = "Light CB";
        cbDesc.Size = sizeof(LightConstantBuffer);
        m_device->CreateBuffer(cbDesc, nullptr, &m_lightCB);

        CameraConstantBuffer defaultCam{};
        defaultCam.View = Matrix4::Identity();
        defaultCam.Projection = Matrix4::Identity();
        UpdateCameraConstantBuffer(defaultCam);

        ObjectConstantBuffer defaultObj{};
        defaultObj.Model = Matrix4::Identity();
        UpdateObjectConstantBuffer(defaultObj);

        LightConstantBuffer defaultLight{};
        UpdateLightConstantBuffer(defaultLight);

        LogInfo("Constant buffers initialized");
    }

    void RTGDRenderSystem::ApplyPendingResize(flecs::world &world) {
        std::lock_guard<std::mutex> lock(m_resizeMutex);
        if (!m_resizePending)
            return;
        m_resizePending = false;

        int width = m_pendingWidth;
        int height = m_pendingHeight;

        if (width <= 0 || height <= 0)
            return;

        m_width = width;
        m_height = height;

        m_pImmediateContext->Flush();
        m_swapChain->Resize(width, height);
        GBufferFactory::Resize(m_gbuffer, *m_device, width, height);

        auto cameraEntity = CameraSystem::GetActiveCamera(world);
        if (cameraEntity.is_valid()) {
            auto cam = cameraEntity.get_ref<CameraComponent>();
            cam->AspectRatio = static_cast<float>(width) / static_cast<float>(height);
        }
    }

    void RTGDRenderSystem::UpdateCameraConstantBuffer(const CameraConstantBuffer &data) {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_pImmediateContext->MapBuffer(m_cameraCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            auto *dst = static_cast<CameraConstantBuffer *>(pMapped);
            dst->View = data.View.Transpose();
            dst->Projection = data.Projection.Transpose();
            dst->CameraPosition = data.CameraPosition;
            m_pImmediateContext->UnmapBuffer(m_cameraCB, MAP_WRITE);
        }
    }

    void RTGDRenderSystem::UpdateObjectConstantBuffer(const ObjectConstantBuffer &data) {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_pImmediateContext->MapBuffer(m_objectCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            auto *dst = static_cast<ObjectConstantBuffer *>(pMapped);
            dst->Model = data.Model.Transpose();
            m_pImmediateContext->UnmapBuffer(m_objectCB, MAP_WRITE);
        }
    }

    void RTGDRenderSystem::Shutdown() {
        LogInfo("Render System Shutdown");

        m_pImmediateContext->Flush();
    }

    void RTGDRenderSystem::RenderGeometry(flecs::world &world) {
        using namespace Diligent;

        auto &rm = RenderResourceManager::Instance();
        TextureHandle def = rm.GetDefaultTextureHandle();
        TextureHandle defNormal = rm.GetDefaultNormalTextureHandle();

        ITextureView *rtvs[] = {
            m_gbuffer.DiffuseRTV,
            m_gbuffer.NormalRTV,
            m_gbuffer.PositionRTV,
            m_gbuffer.PBRRTV
        };
        m_pImmediateContext->SetRenderTargets(
            std::size(rtvs), rtvs, m_gbuffer.DepthDSV,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        m_pImmediateContext->ClearRenderTarget(m_gbuffer.DiffuseRTV, clearColor,
                                               RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearRenderTarget(m_gbuffer.NormalRTV, clearColor,
                                               RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearRenderTarget(m_gbuffer.PositionRTV, clearColor,
                                               RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearRenderTarget(m_gbuffer.PBRRTV, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(
            m_gbuffer.DepthDSV, CLEAR_DEPTH_FLAG, 1.0f, 0,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        world.each([&](const MeshComponent &mesh,
                       const RenderComponent &render,
                       TransformComponent &transform) {
            if (!render.IsVisible)
                return;

            const MeshData &meshData = rm.GetMesh(mesh.meshHandle);
            const MaterialData &gbufMat = rm.GetMaterial(m_gbufferMaterial);

            if (!meshData.VertexBuffer || !gbufMat.PSO || !gbufMat.SRB)
                return;

            const MaterialData &objMat = rm.GetMaterial(mesh.materialHandle);

            auto bindTex = [&](const char *name, TextureHandle handle, TextureHandle fallback) {
                TextureHandle h = (handle != INVALID_TEXTURE_HANDLE) ? handle : fallback;
                if (h == INVALID_TEXTURE_HANDLE)
                    return;

                const TextureData &tex = rm.GetTexture(h);
                if (!tex.SRV)
                    return;

                auto *var = gbufMat.SRB->GetVariableByName(SHADER_TYPE_PIXEL, name);
                if (var)
                    var->Set(tex.SRV, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            };

            bindTex("g_Diffuse", objMat.DiffuseTexture, def);
            bindTex("g_Normal", objMat.NormalTexture, defNormal);
            bindTex("g_MetallicRoughness", objMat.MetallicRoughnessTexture, def);
            bindTex("g_AO", objMat.AOTexture, def);

            if (def != INVALID_TEXTURE_HANDLE) {
                const TextureData &defTex = rm.GetTexture(def);
                auto *samVar = gbufMat.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Sampler");
                if (samVar && defTex.Sampler)
                    samVar->Set(defTex.Sampler, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            }

            m_objectCBData.Model = transform.GetWorldMatrix();
            UpdateObjectConstantBuffer(m_objectCBData);

            m_pImmediateContext->SetPipelineState(gbufMat.PSO);
            m_pImmediateContext->CommitShaderResources(
                gbufMat.SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            IBuffer *vbs[] = {meshData.VertexBuffer};
            Uint64 offsets[] = {0};
            m_pImmediateContext->SetVertexBuffers(
                0, 1, vbs, offsets,
                RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                SET_VERTEX_BUFFERS_FLAG_RESET);

            if (meshData.IndexBuffer && meshData.IndexCount > 0) {
                m_pImmediateContext->SetIndexBuffer(
                    meshData.IndexBuffer, 0,
                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                DrawIndexedAttribs draw;
                draw.NumIndices = meshData.IndexCount;
                draw.IndexType = VT_UINT32;
                draw.Flags = DRAW_FLAG_VERIFY_ALL;
                m_pImmediateContext->DrawIndexed(draw);
            } else {
                DrawAttribs draw;
                draw.NumVertices = meshData.VertexCount;
                draw.Flags = DRAW_FLAG_VERIFY_ALL;
                m_pImmediateContext->Draw(draw);
            }
        });
    }

    void RTGDRenderSystem::RenderLighting() {
        using namespace Diligent;

        auto &rm = RenderResourceManager::Instance();
        const MaterialData &matData = rm.GetMaterial(m_lightingMaterial);

        if (!matData.PSO || !matData.SRB)
            return;

        auto bindSRV = [&](const char *name, ITextureView *srv) {
            auto *var = matData.SRB->GetVariableByName(SHADER_TYPE_PIXEL, name);
            if (var && srv)
                var->Set(srv, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        };

        bindSRV("g_Diffuse", m_gbuffer.DiffuseSRV);
        bindSRV("g_Normal", m_gbuffer.NormalSRV);
        bindSRV("g_Position", m_gbuffer.PositionSRV);
        bindSRV("g_PBR", m_gbuffer.PBRSRV);

        auto defTex = RenderResourceManager::Instance().GetDefaultTextureHandle();
        if (defTex != INVALID_TEXTURE_HANDLE) {
            const auto &tex = RenderResourceManager::Instance().GetTexture(defTex);
            auto *samVar = matData.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Sampler");
            if (samVar && tex.Sampler)
                samVar->Set(tex.Sampler);
        }

        auto *pRTV = m_swapChain->GetCurrentBackBufferRTV();
        auto *pDSV = m_swapChain->GetDepthBufferDSV();

        m_pImmediateContext->SetRenderTargets(
            1, &pRTV, nullptr,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
        m_pImmediateContext->ClearRenderTarget(
            pRTV, clearColor,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pImmediateContext->SetPipelineState(matData.PSO);
        m_pImmediateContext->CommitShaderResources(
            matData.SRB,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs draw;
        draw.NumVertices = 3;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(draw);
    }

    void RTGDRenderSystem::SetActiveCameraCB(flecs::world &world) {
        auto cameraEntity = CameraSystem::GetActiveCamera(world);

        if (cameraEntity.is_valid()) {
            auto *cam = cameraEntity.try_get<CameraComponent>();
            auto *transform = cameraEntity.try_get<TransformComponent>();

            if (!cam || !transform)
                return;

            CameraConstantBuffer cb;
            cb.View = cam->ViewMatrix;
            cb.Projection = cam->ProjectionMatrix;
            cb.CameraPosition =
            {
                transform->Position.x,
                transform->Position.y,
                transform->Position.z, 1.0f
            };

            UpdateCameraConstantBuffer(cb);
        }
    }

    void RTGDRenderSystem::Present() {
        m_swapChain->Present();
    }

    void RTGDRenderSystem::Resize(int width, int height) {
        if (!m_swapChain)
            return;

        std::lock_guard<std::mutex> lock(m_resizeMutex);
        m_resizePending = true;
        m_pendingWidth = width;
        m_pendingHeight = height;
    }

    void RTGDRenderSystem::UpdateLightConstantBuffer(const LightConstantBuffer &data) {
        using namespace Diligent;

        void *pMapped = nullptr;
        m_pImmediateContext->MapBuffer(m_lightCB, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);
        if (pMapped) {
            memcpy(pMapped, &data, sizeof(LightConstantBuffer));
            m_pImmediateContext->UnmapBuffer(m_lightCB, MAP_WRITE);
        }
    }
}
