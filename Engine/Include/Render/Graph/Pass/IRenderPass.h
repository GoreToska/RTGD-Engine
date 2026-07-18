//
// Created by ivan on 7/14/26.
//

#pragma once
#include <vector>

#include "Render/Graph/RGBuilder.h"

namespace Diligent {
    class IRenderDevice;
    class ISwapChain;
}

namespace RTGDEngine {
    class RGBuilder;
    struct RenderContext;
    struct GBuffer;

    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;

        [[nodiscard]] virtual const char *Name() const = 0;

        virtual void Setup(RGBuilder &builder) {
        };

        std::vector<RGAccessDecl> &IO() { return m_io; };

        virtual void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) = 0;

        virtual void Execute(RenderContext &context) = 0;

        [[nodiscard]] virtual bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; };

    protected:
        std::vector<RGAccessDecl> m_io = {};

    private:
        bool m_enabled = true;
    };
}
