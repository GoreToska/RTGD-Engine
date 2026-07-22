//
// Created by ivan on 7/22/26.
//


#include "IRenderPass.h"
#include "Render/RenderHandle.h"

namespace RTGDEngine {
    class CompositePass : public IRenderPass {
    public:
        [[nodiscard]] const char *Name() const override;

        void Execute(RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) override;

        void Setup(RGBuilder &builder) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;

        RGHandle m_sceneColor;
        RGHandle m_backBuffer;
    };
} // RTGDEngine
