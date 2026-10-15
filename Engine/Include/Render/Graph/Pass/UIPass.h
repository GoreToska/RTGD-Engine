//
// Created by ivan on 10/15/26.
//

#pragma once
#include "IRenderPass.h"
#include "AssetLoader/AssetHandle.h"

namespace RTGDEngine
{
    class UIPass : public IRenderPass
    {
    public:
        const char* Name() const override;

        void Initialize(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain) override;

        void Execute(RenderContext& context) override;

        void Setup(RGBuilder& builder) override;

    private:
        RGHandle m_backBuffer;
    };
} // RTGDEngine
