//
// Created by ivan on 7/14/26.
//

#pragma once
#include "IRenderPass.h"

namespace RTGDEngine {
    class CameraPass : public IRenderPass {
    public:
        void Execute(RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) override;

        const char *Name() const override;
    };
} // RTGDEngine
