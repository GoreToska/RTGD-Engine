//
// Created by ivan on 7/14/26.
//

#pragma once

#include <flecs.h>
#include "Render/FrameConstants.h"

#ifdef  RTGD_EDITOR
#include <vector>
#endif

namespace Diligent {
    struct IRenderDevice;
    struct IDeviceContext;
}

namespace RTGDEngine {
    struct RGResources;

    struct RenderContext {
        Diligent::IRenderDevice &Device;
        Diligent::IDeviceContext &Context;
        FrameConstants &Frame;
        flecs::world &World;
        RGResources *Graph = nullptr;
#ifdef  RTGD_EDITOR
        std::vector<flecs::entity> *PickEntities = nullptr; // owned by RenderSystem
#endif
    };
} // RTGDEngine
