//
// Created by ivan on 7/23/26.
//

#pragma once
#include "IRenderPass.h"
#include "Render/RenderHandle.h"

namespace RTGDEngine {
    class ShadowPass : public RTGDEngine::IRenderPass {
    public:
        void Execute(RTGDEngine::RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) override;

        const char *Name() const override;

        void Setup(RTGDEngine::RGBuilder &builder) override;

    private:
        RGHandle m_shadowMap;
        MaterialHandle m_material;
    };
} // RTGDEditor
