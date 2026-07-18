//
// Created by ivan on 7/14/26.
//

#pragma once
#include "Render/RenderHandle.h"
#include "IRenderPass.h"

namespace RTGDEngine {
    class GBufferPass : public IRenderPass {
    public:
        const char *Name() const override;

        void Setup(RGBuilder &builder) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) override;

        void Execute(RenderContext &context) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;

        RGHandle m_diffuse;
        RGHandle m_normal;
        RGHandle m_position;
        RGHandle m_pbr;
        RGHandle m_depth;

#ifdef RTGD_EDITOR
        RGHandle m_id;
#endif
    };
} // RTGDEngine
