//
// Created by ivan on 7/14/26.
//

#pragma once

#include <flecs.h>
#include "Render/FrameConstants.h"
#include "Render/GBuffer.h"

#ifdef  RTGD_EDITOR
#include <vector>
#endif

namespace Diligent {
    struct IRenderDevice;
    struct IDeviceContext;
    struct ISwapChain;
}

namespace RTGDEngine {
    struct RenderContext {
        Diligent::IRenderDevice &Device;
        Diligent::IDeviceContext &Context;
        Diligent::ISwapChain &SwapChain;
        FrameConstants &Frame;
        GBuffer &Gbuffer;
        flecs::world &World;

#ifdef  RTGD_EDITOR
        std::vector<flecs::entity> *PickEntities = nullptr; // owned by RenderSystem
#endif
    };
} // RTGDEngine
