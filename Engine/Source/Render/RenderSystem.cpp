#include "Render/RenderSystem.h"


#include "Render/ShaderLoader.h"

#include <GraphicsTypes.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

#include "Components/CameraComponent.h"
#include "Render/GBufferFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/Graph/RenderContext.h"
#include "Render/Graph/RGResources.h"
#include "Render/Graph/Pass/CameraPass.h"
#include "Render/Graph/Pass/CompositePass.h"
#include "Render/Graph/Pass/DebugViewPass.h"
#include "Render/Graph/Pass/GBufferPass.h"
#include "Render/Graph/Pass/LightPass.h"
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
        m_frameConstants.Initialize(*m_device, *m_pImmediateContext);

        m_gbuffer = GBufferFactory::Create(*m_device, width, height);

        m_graph.AddPass(std::make_unique<CameraPass>());
        m_graph.AddPass(std::make_unique<GBufferPass>());
        m_graph.AddPass(std::make_unique<LightPass>());
        auto debug = std::make_unique<DebugViewPass>();
        debug->SetChannel(EDebugChannel::Normal);
        debug->SetEnabled(false);
        m_graph.AddPass(std::move(debug));
        m_graph.AddPass(std::make_unique<CompositePass>());

        m_graph.Initialize(*m_device, *m_swapChain, m_gbuffer);

        LogInfo("Render system initialized.");
        return true;
    }

    void RTGDRenderSystem::ExecuteFrame(flecs::world &world) {
        RGResources resources(*m_swapChain);
        resources.ImportBackbuffer();
        resources.ImportSwapchainDepth();

        RenderContext renderCtx = {
            *m_device, *m_pImmediateContext, m_frameConstants, world, &resources
        };

#ifdef RTGD_EDITOR
        renderCtx.PickEntities = &m_pickEntities;
#endif
        m_graph.Execute(renderCtx);
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
        m_graph.InvalidateTransientResources();

        auto cameraEntity = CameraSystem::GetActiveCamera(world);
        if (cameraEntity.is_valid()) {
            auto cam = cameraEntity.get_ref<CameraComponent>();
            cam->AspectRatio = static_cast<float>(width) / static_cast<float>(height);
        }
    }


#ifdef RTGD_EDITOR
    flecs::entity RTGDRenderSystem::PickEntity(uint32_t x, uint32_t y) {
        using namespace Diligent;
        if (!m_idReadbackTexture) {
            TextureDesc d;
            d.Name = "GBuffer ID Readback";
            d.Type = RESOURCE_DIM_TEX_2D;
            d.Width = 1;
            d.Height = 1;
            d.MipLevels = 1;
            d.Format = TEX_FORMAT_R32_UINT;
            d.BindFlags = BIND_NONE;
            d.Usage = USAGE_STAGING;
            d.CPUAccessFlags = CPU_ACCESS_READ;
            m_device->CreateTexture(d, nullptr, &m_idReadbackTexture);
        }

        auto *idTex = m_graph.FindTexture("GBuffer.ID");
        if (!idTex) {
            return flecs::entity::null();
        }

        if (x >= m_width || y >= m_height) {
            return flecs::entity::null();
        }

        Box srcBox;
        srcBox.MinX = x;
        srcBox.MaxX = x + 1;
        srcBox.MinY = y;
        srcBox.MaxY = y + 1;
        srcBox.MinZ = 0;
        srcBox.MaxZ = 1;

        CopyTextureAttribs copy;
        copy.pSrcTexture = idTex;
        copy.pSrcBox = &srcBox;
        copy.SrcTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copy.pDstTexture = m_idReadbackTexture;
        copy.DstX = 0;
        copy.DstY = 0;
        copy.DstZ = 0;
        copy.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        m_pImmediateContext->CopyTexture(copy);

        if (!m_pickFence) {
            FenceDesc fd;
            fd.Name = "Pick Fence";
            m_device->CreateFence(fd, &m_pickFence);
        }

        ++m_pickFenceValue;
        m_pImmediateContext->EnqueueSignal(m_pickFence, m_pickFenceValue);
        m_pImmediateContext->Flush();

        m_pickFence->Wait(m_pickFenceValue);

        MappedTextureSubresource mapped;
        m_pImmediateContext->MapTextureSubresource(
            m_idReadbackTexture, 0, 0,
            MAP_READ, MAP_FLAG_DO_NOT_WAIT, nullptr, mapped);

        const uint32_t id = mapped.pData ? *static_cast<const uint32_t *>(mapped.pData) : 0;

        m_pImmediateContext->UnmapTextureSubresource(m_idReadbackTexture, 0, 0);

        if (id == 0 || id > m_pickEntities.size())
            return {};

        return m_pickEntities[id - 1];
    }
#endif

    void RTGDRenderSystem::Shutdown() {
        LogInfo("Render System Shutdown");

        m_pImmediateContext->Flush();
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
}
