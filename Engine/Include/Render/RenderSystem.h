#pragma once

#include <memory>
#include <Windows.h>

#include <RefCntAutoPtr.hpp>

#include <RenderDevice.h>

#include "EngineFactoryD3D12.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine
{
    class ENGINE_API RTGDRenderSystem
    {
    public:
        bool Initialize(void* hwnd, int width, int height);

        void Shutdown();

        void BeginFrame();

        void EndFrame();

        void Resize(int width, int height);

        Diligent::IRenderDevice* GetDevice() const { return m_pDevice; }
        Diligent::IDeviceContext* GetContext() const { return m_pImmediateContext; }
        Diligent::ISwapChain* GetSwapChain() const { return m_pSwapChain; }

    private:
        int m_width = 0;
        int m_height = 0;
        bool m_initialized = false;

        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pTrianglePSO;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pTriangleVB;

        Diligent::IEngineFactoryD3D12* m_pFactory = nullptr;

        void CreateTrianglePipeline();

        void CreateTriangleVertexBuffer();

        void CreateShaders();
    };
}
