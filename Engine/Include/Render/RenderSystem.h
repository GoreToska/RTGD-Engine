#pragma once

#include <memory>
#include <Windows.h>

#include <RefCntAutoPtr.hpp>
#include <flecs.h>

#include <RenderDevice.h>

#include "EngineFactoryD3D12.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine
{
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
    public:
        static RTGDRenderSystem& Instance();

        bool Initialize(HWND hwnd, int width, int height);

        void BeginFrame();

        void RenderScene(flecs::world& world);

        void EndFrame();

        void Shutdown();

        void Resize(int width, int height);

        [[nodiscard]] Diligent::IRenderDevice& GetDevice() const { return *m_pDevice; }
        [[nodiscard]] Diligent::IDeviceContext& GetContext() const { return *m_pImmediateContext; }
        [[nodiscard]] Diligent::ISwapChain& GetSwapChain() const { return *m_pSwapChain; }
        [[nodiscard]] Diligent::IEngineFactoryD3D12& GetFactory() const { return *m_pFactory; }
        [[nodiscard]] Diligent::IBuffer& GetCameraCB() const { return *m_cameraCB; }
        [[nodiscard]] Diligent::IBuffer& GetObjectCB() const { return *m_objectCB; }

    private:
        int m_width = 0;
        int m_height = 0;
        bool m_initialized = false;

        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;

        Diligent::IEngineFactoryD3D12* m_pFactory = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cameraCB;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_objectCB;

        void CreateShaders();

        void InitializeConstantBuffers();

        void UpdateCameraConstantBuffer(const CameraConstantBuffer& data);

        void UpdateObjectConstantBuffer(const ObjectConstantBuffer& data);
    };
}
