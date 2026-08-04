//
// Created by ivan on 7/14/26.
//

#pragma once
#include "Render/RenderHandle.h"
#include "IRenderPass.h"

namespace RTGDEngine {
    class LightPass : public IRenderPass {
    public:
        const char *Name() const override;

        void Setup(RGBuilder &builder) override;

        void Execute(RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) override;

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;
        Diligent::TEXTURE_FORMAT m_colorFormat = Diligent::TEX_FORMAT_UNKNOWN;

        RGHandle m_diffuse;
        RGHandle m_normal;
        RGHandle m_position;
        RGHandle m_pbr;
        RGHandle m_shadowMap;
        RGHandle m_depth;
        RGHandle m_sceneColor;
    };
} // RTGDEngine
