#pragma once

#include <memory>

#include <RefCntAutoPtr.hpp>
#include <flecs.h>
#include <EngineFactory.h>
#include <RenderDevice.h>

#ifdef _WIN32
#include <EngineFactoryD3D12.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <EngineFactoryVk.h>
#endif


#include "ConstBuffers.h"
#include "GBuffer.h"
#include "RenderHandle.h"
#include "Engine/EngineExport.h"
#include "Platform/WindowHandle.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    struct LightConstantBuffer;
    struct CameraConstantBuffer;
}

namespace RTGDEngine
{
    struct ObjectConstantBuffer;
}

namespace RTGDEngine
{
    class ENGINE_API RTGDRenderSystem
    {
        DECLARE_SINGLETON(RTGDRenderSystem);

    public:
        bool Initialize(const NativeWindowHandle& handle, int width, int height);

        void RenderLighting();

        void SetActiveCameraCB(flecs::world& world);

        void RenderGeometry(flecs::world& world);

        void Present();

        void Shutdown();

        void Resize(int width, int height);

        void ApplyPendingResize(flecs::world& world);

        [[nodiscard]] Diligent::IRenderDevice& GetDevice() const { return *m_device; }
        [[nodiscard]] Diligent::IDeviceContext& GetContext() const { return *m_pImmediateContext; }
        [[nodiscard]] Diligent::ISwapChain& GetSwapChain() const { return *m_swapChain; }
        [[nodiscard]] Diligent::IEngineFactory& GetFactory() const { return *m_pFactory; }
        [[nodiscard]] Diligent::IBuffer& GetCameraCB() const { return *m_cameraCB; }
        [[nodiscard]] Diligent::IBuffer& GetObjectCB() const { return *m_objectCB; }
        [[nodiscard]] Diligent::IBuffer& GetLightCB() const { return *m_lightCB; }
        [[nodiscard]] const GBuffer& GetGBuffer() const { return m_gbuffer; }

        void UpdateLightConstantBuffer(const LightConstantBuffer& data);

        void UpdateCameraConstantBuffer(const CameraConstantBuffer& data);

        void UpdateObjectConstantBuffer(const ObjectConstantBuffer& data);

    private:
        bool m_initialized = false;

        std::mutex m_resizeMutex;
        int m_width = 0;
        int m_height = 0;
        bool m_resizePending = false;
        int m_pendingWidth = 0;
        int m_pendingHeight = 0;

        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_device;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_swapChain;

        Diligent::IEngineFactory* m_pFactory = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cameraCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_objectCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightCB;

        GBuffer m_gbuffer;

        CameraConstantBuffer m_cameraCBData{};
        ObjectConstantBuffer m_objectCBData{};

        MaterialHandle m_gbufferMaterial = INVALID_MATERIAL_HANDLE;
        MaterialHandle m_lightingMaterial = INVALID_MATERIAL_HANDLE;

        void InitializeConstantBuffers();
    };
}
