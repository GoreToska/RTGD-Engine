//
// Created by ivan on 7/14/26.
//

#pragma once

#include <flecs.h>
#include <span>

#include "Render/FrameConstants.h"
#include "Render/RenderScene.h"
#include "Render/RenderView.h"

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
        const RenderScene *Scene = nullptr;
        const RenderView *MainView = nullptr;
        std::span<const RenderView> ShadowViews;
    };
} // RTGDEngine
