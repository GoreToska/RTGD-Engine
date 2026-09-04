//
// Created by ivan on 8/27/26.
//

#pragma once
#include "IRenderPass.h"
#include "Render/RenderHandle.h"


namespace RTGDEngine
{
    class DebugLinesPass : public IRenderPass
    {
    public:
        void Execute(RenderContext& context) override;

        void Initialize(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain) override;

        [[nodiscard]] const char* Name() const override;

        void Setup(RGBuilder& builder) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;

        RGHandle m_backBuffer;
        RGHandle m_depth;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vertexBuffer;
        uint32_t m_vertexCapacity = 0;
    };
} // RTGDEngine
