//
// Created by ivan on 7/14/26.
//

#pragma once
#include <cstdint>

#include "IRenderPass.h"
#include "Render/RenderHandle.h"

namespace RTGDEngine {
    enum class EDebugChannel : uint32_t {
        Diffuse,
        Normal,
        Position,
        PBR,
        Depth
    };

    class DebugViewPass : public IRenderPass {
    public:
        DebugViewPass();

        void Setup(RGBuilder &builder) override;

        void Execute(RenderContext &context) override;

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) override;

        const char *Name() const override;

        void SetChannel(EDebugChannel channel) { m_channel = channel; };

    private:
        MaterialHandle m_material = INVALID_MATERIAL_HANDLE;
        EDebugChannel m_channel = EDebugChannel::Diffuse;
        RGHandle m_texture;
        RGHandle m_backbuffer;
    };
} // RTGDEngine
