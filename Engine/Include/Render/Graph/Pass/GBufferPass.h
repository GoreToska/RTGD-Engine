//
// Created by ivan on 7/14/26.
//

#pragma once
#include "Render/RenderHandle.h"
#include "Render/Graph/IRenderPass.h"

namespace RTGDEngine {
    class GBufferPass : public IRenderPass {
    public:
        const char *Name() const override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) override;

        void Execute(RenderContext &context) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;
    };
} // RTGDEngine
