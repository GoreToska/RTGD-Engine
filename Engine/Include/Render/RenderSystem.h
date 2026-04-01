#pragma once

#include <memory>
#include <Windows.h>

#include <RefCntAutoPtr.hpp>
#include <flecs.h>

#include <RenderDevice.h>

#include "ConstBuffers.h"
#include "EngineFactoryD3D12.h"
#include "GBuffer.h"
#include "RenderHandle.h"
#include "Engine/EngineExport.h"
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
        bool Initialize(HWND hwnd, int width, int height);

        void RenderLighting();

        void SetActiveCameraCB(flecs::world& world);

        void RenderGeometry(flecs::world& world);

        void Present();

        void Shutdown();

        void Resize(int width, int height);

        void ApplyPendingResize();

        [[nodiscard]] Diligent::IRenderDevice& GetDevice() const { return *m_device; }
        [[nodiscard]] Diligent::IDeviceContext& GetContext() const { return *m_pImmediateContext; }
        [[nodiscard]] Diligent::ISwapChain& GetSwapChain() const { return *m_swapChain; }
        [[nodiscard]] Diligent::IEngineFactoryD3D12& GetFactory() const { return *m_pFactory; }
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

        Diligent::IEngineFactoryD3D12* m_pFactory = nullptr;

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
