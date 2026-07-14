//
// Created by ivan on 7/14/26.
//

#pragma once
#include "Render/RenderHandle.h"
#include "Render/Graph/IRenderPass.h"

namespace RTGDEngine {
    class LightPass : public IRenderPass {
    public:
        const char *Name() const override;

        void Execute(RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;
    };
} // RTGDEngine
