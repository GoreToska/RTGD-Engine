#pragma once

#include <memory>

#include <RefCntAutoPtr.hpp>
#include <flecs.h>
#include <EngineFactory.h>
#include <RenderDevice.h>

#include "FrameConstants.h"
#include "Graph/RenderGraph.h"

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

namespace RTGDEngine {
    struct LightConstantBuffer;
    struct CameraConstantBuffer;
}

namespace RTGDEngine {
    struct ObjectConstantBuffer;
}

namespace RTGDEngine {
    class ENGINE_API RTGDRenderSystem {
        DECLARE_SINGLETON(RTGDRenderSystem);

    public:
        bool Initialize(const NativeWindowHandle &handle, int width, int height);

        void ExecuteFrame(flecs::world &world);

        void Present();

        void Shutdown();

        void Resize(int width, int height);

        void ApplyPendingResize(flecs::world &world);

        [[nodiscard]] Diligent::IRenderDevice &GetDevice() const { return *m_device; }
        [[nodiscard]] Diligent::IDeviceContext &GetContext() const { return *m_pImmediateContext; }
        [[nodiscard]] Diligent::ISwapChain &GetSwapChain() const { return *m_swapChain; }
        [[nodiscard]] Diligent::IEngineFactory &GetFactory() const { return *m_pFactory; }
        [[nodiscard]] FrameConstants &GetFrameConstants() { return m_frameConstants; }
        [[nodiscard]] const GBuffer &GetGBuffer() const { return m_gbuffer; }

#ifdef RTGD_EDITOR
        flecs::entity PickEntity(uint32_t x, uint32_t y);
#endif

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

        Diligent::IEngineFactory *m_pFactory = nullptr;

        FrameConstants m_frameConstants = {};
        GBuffer m_gbuffer = {};
        RenderGraph m_graph = {};

#ifdef RTGD_EDITOR
        std::vector<flecs::entity> m_pickEntities = {};

        Diligent::RefCntAutoPtr<Diligent::IFence> m_pickFence = {};
        Diligent::Uint64 m_pickFenceValue = 0;
#endif
    };
}
