//
// Created by ivan on 7/14/26.
//

#pragma once
namespace Diligent {
    class IRenderDevice;
    class ISwapChain;
}

namespace RTGDEngine {
    struct RenderContext;
    struct GBuffer;

    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;

        [[nodiscard]] virtual const char *Name() const = 0;

        virtual void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) = 0;

        virtual void Execute(RenderContext &context) = 0;
    };
}
